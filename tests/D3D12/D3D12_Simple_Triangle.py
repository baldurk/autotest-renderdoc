import rdtest


class D3D12_Simple_Triangle(rdtest.TestCase):
    platform = 'win32'
    platform_version = 10

    def get_capture(self):
        return rdtest.run_and_capture("demos_x64", "D3D12_Simple_Triangle", 5)

    def check_capture(self):
        self.check_final_backbuffer()

        self.check_export(self.capture_filename)
