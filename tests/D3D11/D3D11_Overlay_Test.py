import rdtest
import renderdoc as rd


class D3D11_Overlay_Test(rdtest.TestCase):
    def get_capture(self):
        return rdtest.run_and_capture("demos_x64.exe", "D3D11_Overlay_Test", 5)

    def check_capture(self):
        self.check_final_backbuffer()

        out: rd.ReplayOutput = self.controller.CreateOutput(rd.CreateHeadlessWindowingData(), rd.ReplayOutputType.Texture)

        self.check(out is not None)

        out.SetDimensions(100, 100)

        test_marker: rd.DrawcallDescription = self.find_draw("Test")

        self.controller.SetFrameEvent(test_marker.children[0].eventId, True)

        pipe: rd.PipeState = self.controller.GetPipelineState()

        tex = rd.TextureDisplay()
        tex.resourceId = pipe.GetOutputTargets()[0].resourceId

        for overlay in rd.DebugOverlay:
            if overlay == rd.DebugOverlay.NoOverlay:
                continue

            # These overlays are just displaymodes really, not actually separate overlays
            if overlay == rd.DebugOverlay.NaN or overlay == rd.DebugOverlay.Clipping:
                continue

            tex.overlay = overlay
            out.SetTextureDisplay(tex)

            overlay_path = rdtest.get_tmp_path(str(overlay) + '.png')
            ref_path = self.get_ref_path(str(overlay) + '.png')

            save_data = rd.TextureSave()
            save_data.resourceId = out.GetDebugOverlayTexID()
            save_data.destType = rd.FileType.PNG

            # These overlays modify the underlying texture, so we need to save it out instead of the overlay
            if overlay == rd.DebugOverlay.ClearBeforeDraw or overlay == rd.DebugOverlay.ClearBeforePass:
                save_data.resourceId = tex.resourceId

            self.controller.SaveTexture(save_data, overlay_path)

            if not rdtest.image_compare(overlay_path, ref_path):
                raise rdtest.TestFailureException("Reference and output image differ for overlay {}".format(str(overlay)), overlay_path, ref_path)

            rdtest.log.success("Reference and output image are identical for {}".format(str(overlay)))

        out.Shutdown()