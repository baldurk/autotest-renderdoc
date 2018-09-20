import os
import renderdoc as rd
from . import util
from . import analyse


class TestCase:
    def __init__(self):
        self.capture_filename = ""

    def get_ref_path(self, name: str):
        return os.path.join(os.getcwd(), 'data', self.__class__.__name__, name)

    def get_capture(self):
        """
        Method to overload if not implementing a run(), using the default run which
        handles everything and calls get_capture() and check_capture() for you.

        :return: The path to the capture to open. If in a temporary path, it will be
          deleted if the test completes.
        """
        raise NotImplementedError("If run() is not implemented in a test, then"
                                  "get_capture() and check_capture() must be.")

    def check_capture(self, controller):
        """
        Method to overload if not implementing a run(), using the default run which
        handles everything and calls get_capture() and check_capture() for you.
        """
        raise NotImplementedError("If run() is not implemented in a test, then"
                                  "get_capture() and check_capture() must be.")

    def run(self):
        self.capture_filename = self.get_capture()

        if not os.path.exists(self.capture_filename):
            raise RuntimeError("Didn't generate capture in make_capture")

        print("Loading capture...")

        controller = analyse.open_capture(self.capture_filename)

        print("Checking capture...")

        self.check_capture(controller)

        controller.Shutdown()

    def check_final_backbuffer(self, controller: rd.ReplayController):
        img_path = util.get_tmp_path('backbuffer.png')
        ref_path = self.get_ref_path('backbuffer.png')

        last_draw: rd.DrawcallDescription = controller.GetDrawcalls()[-1]

        controller.SetFrameEvent(last_draw.eventId, True)

        save_data = rd.TextureSave()
        save_data.resourceId = last_draw.copyDestination
        save_data.destType = rd.FileType.PNG

        controller.SaveTexture(save_data, img_path)

        if not util.image_compare(img_path, ref_path):
            raise RuntimeError("Reference and output backbuffer image differ")

        print("Backbuffer is identical to reference")

    def check_export(self, capture_filename):
        xml_out_path = util.get_tmp_path('export.xml')
        xml_ref_path = self.get_ref_path('export.xml')

        recomp_path = util.get_tmp_path('recompressed.rdc')
        conv_zipxml_path = util.get_tmp_path('conv.zip.xml')
        conv_path = util.get_tmp_path('conv.rdc')

        origrdc = rd.OpenCaptureFile()
        status = origrdc.OpenFile(capture_filename, '', None)

        if status != rd.ReplayStatus.Succeeded:
            raise RuntimeError("Couldn't open '{}': {}".format(capture_filename, str(status)))

        sdfile: rd.SDFile = origrdc.GetStructuredData()

        # Use the same buffers & version
        stripped_sdfile = rd.SDFile()
        stripped_sdfile.version = sdfile.version

        # Copy the chunks but replace metadata with deterministic values
        c: rd.SDChunk
        timestamp = 123450
        for c in sdfile.chunks:
            chunk: rd.SDChunk = c.Duplicate()
            chunk.metadata.durationMicro = 5
            chunk.metadata.timestampMicro = timestamp
            chunk.metadata.threadID = 999
            timestamp += 10
            stripped_sdfile.chunks.append(chunk)

        origrdc.Convert(xml_out_path, 'xml', stripped_sdfile, None)

        if not util.md5_compare(xml_out_path, xml_ref_path):
            raise RuntimeError("Reference and output XML differ")

        print("Exported XML format is identical to reference")

        # Export to rdc, to recompress
        origrdc.Convert(recomp_path, '', None, None)
        origrdc.Convert(conv_zipxml_path, 'zip.xml', None, None)

        origrdc.Shutdown()

        # Load up the zip.xml file
        zipxml = rd.OpenCaptureFile()
        status = zipxml.OpenFile(conv_zipxml_path, 'zip.xml', None)

        if status != rd.ReplayStatus.Succeeded:
            raise RuntimeError("Couldn't open '{}': {}".format(conv_zipxml_path, str(status)))

        # Convert out to rdc
        zipxml.Convert(conv_path, '', None, None)

        zipxml.Shutdown()

        if not util.md5_compare(recomp_path, conv_path):
            raise RuntimeError("Recompressed capture file doesn't match re-imported capture file")

        print("Recompressed and re-imported capture files are identical")
