import rdtest


class GL_Simple_Triangle(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "GL_Simple_Triangle", 5)

    def check_capture(self):
        self.check_final_backbuffer()

        self.check_export(self.capture_filename)
