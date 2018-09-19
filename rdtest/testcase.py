import os
import renderdoc as rd
from . import util
from . import analyse


class TestCase:
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
        capture_filename = self.get_capture()

        if not os.path.exists(capture_filename):
            raise RuntimeError("Didn't generate capture in make_capture")

        print("Loading capture...")

        controller = analyse.open_capture(capture_filename)

        print("Checking capture...")

        self.check_capture(controller)

        controller.Shutdown()

    def check_final_backbuffer(self, controller):
        last_draw: rd.DrawcallDescription = controller.GetDrawcalls()[-1]

        controller.SetFrameEvent(last_draw.eventId, True)

        save_data = rd.TextureSave()
        save_data.resourceId = last_draw.copyDestination
        save_data.destType = rd.FileType.PNG

        img_path = util.get_tmp_path('backbuffer.png')
        ref_path = self.get_ref_path('backbuffer.png')

        controller.SaveTexture(save_data, img_path)

        if not util.image_compare(img_path, ref_path):
            raise RuntimeError("Reference image and output image differ")