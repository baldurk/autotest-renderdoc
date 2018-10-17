import struct
from typing import List
import renderdoc

# Alias for convenience - we need to import as-is so types don't get confused
rd = renderdoc


def open_capture(filename="", cap: rd.CaptureFile=None):
    """
    Opens a capture file and begins a replay.

    :param filename: The filename to open, or empty if cap is used.
    :param cap: The capture file to use, or ``None`` if a filename is given.
    :return: A replay controller for the capture
    :rtype: renderdoc.ReplayController
    """

    # Open a capture file handle
    own_cap = False

    if cap is None:
        own_cap = True

        cap = rd.OpenCaptureFile()

        # Open a particular file
        status = cap.OpenFile(filename, '', None)

        # Make sure the file opened successfully
        if status != rd.ReplayStatus.Succeeded:
            cap.Shutdown()
            raise RuntimeError("Couldn't open '{}': {}".format(filename, str(status)))

        # Make sure we can replay
        if not cap.LocalReplaySupport():
            cap.Shutdown()
            raise RuntimeError("Capture cannot be replayed")

    status, controller = cap.OpenCapture(None)

    if own_cap:
        cap.Shutdown()

    if status != rd.ReplayStatus.Succeeded:
        raise RuntimeError("Couldn't initialise replay: {}".format(str(status)))

    return controller


def fetch_indices(controller: rd.ReplayController, mesh: rd.MeshFormat, index_offset: int, first_index: int, num_indices: int):
    # Get the character for the width of index
    index_fmt = 'B'
    if mesh.indexByteStride == 2:
        index_fmt = 'H'
    elif mesh.indexByteStride == 4:
        index_fmt = 'I'

    # Duplicate the format by the number of indices
    index_fmt = str(num_indices) + index_fmt

    # If we have an index buffer
    if mesh.indexResourceId != rd.ResourceId.Null():
        # Fetch the data
        ibdata = controller.GetBufferData(mesh.indexResourceId,
                                          mesh.indexByteOffset + mesh.indexByteStride*(first_index + index_offset),
                                          mesh.indexByteStride*num_indices)

        # Unpack all the indices
        indices = struct.unpack(index_fmt, ibdata)

        # Apply the baseVertex offset
        return [i + mesh.baseVertex for i in indices]
    else:
        # With no index buffer, just generate a range
        return tuple(range(first_index, first_index + num_indices))


class MeshAttribute:
    mesh: rd.MeshFormat
    name: str


def get_postvs_attrs(controller: rd.ReplayController, mesh: rd.MeshFormat, data_stage: rd.MeshDataStage):
    pipe: rd.PipeState = controller.GetPipelineState()

    if data_stage == rd.MeshDataStage.VSOut:
        shader = pipe.GetShaderReflection(rd.ShaderStage.Vertex)
    else:
        shader = pipe.GetShaderReflection(rd.ShaderStage.Geometry)
        if shader is None:
            shader = pipe.GetShaderReflection(rd.ShaderStage.Domain)

    attrs: List[MeshAttribute] = []
    posidx = 0

    for sig in shader.outputSignature:
        attr = MeshAttribute()
        attr.mesh = rd.MeshFormat(mesh)

        # Construct a resource format for this element
        attr.mesh.format = rd.ResourceFormat()
        attr.mesh.format.compByteWidth = 8 if sig.compType == rd.CompType.Double else 4
        attr.mesh.format.compCount = sig.compCount
        attr.mesh.format.compType = sig.compType
        attr.mesh.format.type = rd.ResourceFormatType.Regular

        attr.name = sig.semanticIdxName if sig.varName == '' else sig.varName

        if sig.systemValue == rd.ShaderBuiltin.Position:
            posidx = len(attrs)

        attrs.append(attr)

    # Shuffle the position element to the front
    if posidx > 0:
        pos = attrs[posidx]
        del attrs[posidx]
        attrs.insert(0, pos)

    accum_offset = 0

    for i in range(0, len(attrs)):
        attrs[i].mesh.vertexByteOffset = accum_offset

        # Note that some APIs such as Vulkan will pad the size of the attribute here
        # while others will tightly pack
        fmt = attrs[i].mesh.format

        accum_offset += (8 if fmt.compType == rd.CompType.Double else 4) * fmt.compCount

        if pipe.HasAlignedPostVSData(data_stage) and (accum_offset % 16) != 0:
            accum_offset += 16 - (accum_offset % 16)

    return attrs


# Unpack a tuple of the given format, from the data
def unpack_data(fmt: rd.ResourceFormat, data: bytes, data_offset: int):
    # We don't handle 'special' formats - typically bit-packed such as 10:10:10:2
    if fmt.Special():
        raise RuntimeError("Packed formats are not supported!")

    format_chars = {
        #                   012345678
        rd.CompType.UInt:  "xBHxIxxxL",
        rd.CompType.SInt:  "xbhxixxxl",
        rd.CompType.Float: "xxexfxxxd",  # only 2, 4 and 8 are valid
    }

    # These types have identical decodes, but we might post-process them
    format_chars[rd.CompType.UNorm] = format_chars[rd.CompType.UInt]
    format_chars[rd.CompType.UScaled] = format_chars[rd.CompType.UInt]
    format_chars[rd.CompType.SNorm] = format_chars[rd.CompType.SInt]
    format_chars[rd.CompType.SScaled] = format_chars[rd.CompType.SInt]
    format_chars[rd.CompType.Double] = format_chars[rd.CompType.Float]

    # We need to fetch compCount components
    vertex_format = str(fmt.compCount) + format_chars[fmt.compType][fmt.compByteWidth]

    # Unpack the data
    try:
        value = struct.unpack_from(vertex_format, data, data_offset)
    except struct.error as ex:
        raise

    # If the format needs post-processing such as normalisation, do that now
    if fmt.compType == rd.CompType.UNorm:
        divisor = float((1 << fmt.compByteWidth) - 1)
        value = tuple(float(value[i]) / divisor for i in value)
    elif fmt.compType == rd.CompType.SNorm:
        max_neg = -(1 << (fmt.compByteWidth - 1))
        divisor = float(-(max_neg-1))
        value = tuple((float(value[i]) if (value[i] == max_neg) else (float(value[i]) / divisor)) for i in value)

    # If the format is BGRA, swap the two components
    if fmt.bgraOrder:
        value = tuple(value[i] for i in [2, 1, 0, 3])

    return value


def decode_mesh_data(controller: rd.ReplayController, indices: List[int], attrs: List[MeshAttribute], instance: int=0):
    buffer_cache = {}
    ret = []

    # Calculate the strip restart index for this index width
    striprestart_index = None
    if controller.GetPipelineState().IsStripRestartEnabled():
        striprestart_index = (controller.GetPipelineState().GetStripRestartIndex() &
                              ((1 << (attrs[0].mesh.indexByteStride*8)) - 1))

    for i,idx in enumerate(indices):
        vertex = {'vtx': i, 'idx': idx}

        if striprestart_index is None or idx != striprestart_index:
            for attr in attrs:
                offset = attr.mesh.vertexByteOffset + attr.mesh.vertexByteStride * idx

                if attr.mesh.instanced:
                    offset = (attr.mesh.vertexByteStride +
                              attr.mesh.vertexByteStride * (instance / max(attr.mesh.instStepRate, 1)))

                # This could be more optimal if we figure out the lower/upper bounds of any attribute and only fetch the
                # data we need.
                if attr.mesh.vertexResourceId not in buffer_cache:
                    buffer_cache[attr.mesh.vertexResourceId] = controller.GetBufferData(attr.mesh.vertexResourceId, 0, 0)

                vertex[attr.name] = unpack_data(attr.mesh.format, buffer_cache[attr.mesh.vertexResourceId], offset)

        ret.append(vertex)

    return ret
