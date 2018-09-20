import rdtest as rdtest


class D3D11SimpleTriangle(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "D3D11_Simple_Triangle", 5)

    def check_capture(self, controller):
        self.check_final_backbuffer(controller)

        self.check_export(self.capture_filename)
