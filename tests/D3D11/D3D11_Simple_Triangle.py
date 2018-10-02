import rdtest


class D3D11_Simple_Triangle(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "D3D11_Simple_Triangle", 5, "D3D11_Simple_Triangle")

    def check_capture(self):
        self.check_final_backbuffer()

        self.check_export(self.capture_filename)