import rdtest as rdtest


class GLSimpleTriangle(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "GL_Simple_Triangle", 5)

    def check_capture(self, controller):
        self.check_final_backbuffer(controller)

        self.check_export(self.capture_filename)
