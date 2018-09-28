import os
import renderdoc as rd
import traceback
import re
from . import util
from . import analyse
from .logging import log, TestFailureException


class TestCase:
    def __init__(self):
        self.capture_filename = ""
        self.controller: rd.ReplayController = None

    def get_ref_path(self, name: str):
        return os.path.join(util.get_root_dir(), 'data', self.__class__.__name__, name)

    def check(self, expr, msg=None):
        if not expr:
            callstack = traceback.extract_stack()
            callstack.pop()
            assertion_line = callstack[-1].line

            assert_msg = re.sub(r'[^(]*\(([^,]*)(,.*)?\).*', r'\1', assertion_line)

            if msg is None:
                raise TestFailureException('Assertion Failure: {}'.format(assert_msg))
            else:
                raise TestFailureException('{}: {}'.format(msg, assert_msg))

    def get_capture(self):
        """
        Method to overload if not implementing a run(), using the default run which
        handles everything and calls get_capture() and check_capture() for you.

        :return: The path to the capture to open. If in a temporary path, it will be
          deleted if the test completes.
        """
        raise NotImplementedError("If run() is not implemented in a test, then"
                                  "get_capture() and check_capture() must be.")

    def check_capture(self):
        """
        Method to overload if not implementing a run(), using the default run which
        handles everything and calls get_capture() and check_capture() for you.
        """
        raise NotImplementedError("If run() is not implemented in a test, then"
                                  "get_capture() and check_capture() must be.")

    def _find_draw(self, name: str, start_event: int, draw_list):
        draw: rd.DrawcallDescription
        for draw in draw_list:
            # If this draw matches, return it
            if draw.eventId >= start_event and name in draw.name:
                return draw

            # Recurse to children - depth-first search
            ret: rd.DrawcallDescription = self._find_draw(name, start_event, draw.children)

            # If we found our draw, return
            if ret is not None:
                return ret

            # Otherwise continue to next in the list

        # If we didn't find anything, return None
        return None

    def find_draw(self, name: str, start_event: int = 0):
        """
        Finds the first drawcall matching given criteria

        :param name: The name to search for within the drawcalls
        :param start_event: The first eventId to search from.
        :return:
        """

        return self._find_draw(name, start_event, self.controller.GetDrawcalls())

    def run(self):
        self.capture_filename = self.get_capture()

        self.check(os.path.exists(self.capture_filename), "Didn't generate capture in make_capture")

        log.print("Loading capture")

        self.controller = analyse.open_capture(self.capture_filename)

        log.print("Checking capture")

        self.check_capture()

        self.controller.Shutdown()

    def invoketest(self):
        self.run()

    def check_final_backbuffer(self):
        img_path = util.get_tmp_path('backbuffer.png')
        ref_path = self.get_ref_path('backbuffer.png')

        last_draw: rd.DrawcallDescription = self.controller.GetDrawcalls()[-1]

        self.controller.SetFrameEvent(last_draw.eventId, True)

        save_data = rd.TextureSave()
        save_data.resourceId = last_draw.copyDestination
        save_data.destType = rd.FileType.PNG

        self.controller.SaveTexture(save_data, img_path)

        if not util.image_compare(img_path, ref_path):
            raise TestFailureException("Reference and output backbuffer image differ", img_path, ref_path)

        log.success("Backbuffer is identical to reference")

    def check_export(self, capture_filename):
        xml_out_path = util.get_tmp_path('export.xml')
        xml_ref_path = self.get_ref_path('export.xml')

        recomp_path = util.get_tmp_path('recompressed.rdc')
        conv_zipxml_path = util.get_tmp_path('conv.zip.xml')
        conv_path = util.get_tmp_path('conv.rdc')

        origrdc = rd.OpenCaptureFile()
        status = origrdc.OpenFile(capture_filename, '', None)

        self.check(status == rd.ReplayStatus.Succeeded, "Couldn't open '{}': {}".format(capture_filename, str(status)))

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
            raise TestFailureException("Reference and output XML differ", xml_out_path, xml_ref_path)

        log.success("Exported XML format is identical to reference")

        # Export to rdc, to recompress
        origrdc.Convert(recomp_path, '', None, None)
        origrdc.Convert(conv_zipxml_path, 'zip.xml', None, None)

        origrdc.Shutdown()

        # Load up the zip.xml file
        zipxml = rd.OpenCaptureFile()
        status = zipxml.OpenFile(conv_zipxml_path, 'zip.xml', None)

        self.check(status == rd.ReplayStatus.Succeeded, "Couldn't open '{}': {}".format(conv_zipxml_path, str(status)))

        # Convert out to rdc
        zipxml.Convert(conv_path, '', None, None)

        zipxml.Shutdown()

        if not util.md5_compare(recomp_path, conv_path):
            raise TestFailureException("Recompressed capture file doesn't match re-imported capture file",
                                       recomp_path, conv_path)

        log.success("Recompressed and re-imported capture files are identical")
