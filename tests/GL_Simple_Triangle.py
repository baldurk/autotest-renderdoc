import rdtest as rdtest


class GL_Simple_Triangle(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "GL_Simple_Triangle", 5, "GL_Simple_Triangle")

    def check_capture(self, controller):
        self.check_final_backbuffer(controller)

        self.check_export(self.capture_filename)
