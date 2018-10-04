/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "d3d11_test.h"

struct D3D11_Overlay_Test : D3D11GraphicsTest
{
  static constexpr char *Description =
      "Makes a couple of draws that show off all the overlays in some way";

  int main(int argc, char **argv)
  {
    // initialise, create window, create device, etc
    if(!Init(argc, argv))
      return 3;

    ID3DBlobPtr vsblob = Compile(DefaultVertex, "main", "vs_4_0");
    ID3DBlobPtr psblob = Compile(DefaultPixel, "main", "ps_4_0");

    CreateDefaultInputLayout(vsblob);

    ID3D11VertexShaderPtr vs = CreateVS(vsblob);
    ID3D11PixelShaderPtr ps = CreatePS(psblob);

    const DefaultA2V VBData[] = {
        // this triangle occludes in depth
        {Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(-0.5f, 0.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.0f, 0.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // this triangle occludes in stencil
        {Vec3f(-0.5f, 0.0f, 0.9f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(-0.5f, 0.5f, 0.9f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.0f, 0.9f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // this triangle is just in the background to contribute to overdraw
        {Vec3f(-0.9f, -0.9f, 0.95f), Vec4f(0.1f, 0.1f, 0.1f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.9f, 0.95f), Vec4f(0.1f, 0.1f, 0.1f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.9f, -0.9f, 0.95f), Vec4f(0.1f, 0.1f, 0.1f, 1.0f), Vec2f(1.0f, 0.0f)},

        // the draw has a few triangles, main one that is occluded for depth, another that is
        // adding to overdraw complexity, one that is backface culled, then a few more of various
        // sizes for triangle size overlay
        {Vec3f(-0.3f, -0.5f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(-0.3f, 0.5f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.5f, 0.0f, 0.5f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        {Vec3f(-0.2f, -0.2f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.2f, 0.0f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.2f, -0.4f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // backface culled
        {Vec3f(0.1f, 0.0f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.5f, -0.2f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.5f, 0.2f, 0.5f), Vec4f(0.0f, 0.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // small triangles
        // size=0.005
        {Vec3f(0.0f, 0.3f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.305f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.005f, 0.3f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // size=0.01
        {Vec3f(0.0f, 0.4f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.41f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.01f, 0.4f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // size=0.015
        {Vec3f(0.0f, 0.5f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.515f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.015f, 0.5f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},

        // size=0.02
        {Vec3f(0.0f, 0.6f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
        {Vec3f(0.0f, 0.62f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
        {Vec3f(0.02f, 0.6f, 0.5f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(1.0f, 0.0f)},
    };

    ID3D11BufferPtr vb = MakeBuffer().Vertex().Data(VBData);

    ID3D11Texture2DPtr depthtex =
        MakeTexture(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, screenWidth, screenHeight).DSV();
    ID3D11DepthStencilViewPtr dsv = MakeDSV(depthtex);

    while(Running())
    {
      ClearRenderTargetView(bbRTV, {0.4f, 0.5f, 0.6f, 1.0f});
      ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

      IASetVertexBuffer(vb, sizeof(DefaultA2V), 0);
      ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      ctx->IASetInputLayout(defaultLayout);

      ctx->VSSetShader(vs, NULL, 0);
      ctx->PSSetShader(ps, NULL, 0);

      D3D11_DEPTH_STENCIL_DESC depth = GetDepthState();

      depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
      depth.StencilEnable = FALSE;
      depth.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
      depth.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

      SetDepthState(depth);
      SetStencilRef(0x55);

      RSSetViewport({0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f});

      ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), dsv);

      // draw the setup triangles

      // 1: write depth
      depth.DepthFunc = D3D11_COMPARISON_ALWAYS;
      SetDepthState(depth);
      ctx->Draw(3, 0);

      // 2: write stencil
      depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
      depth.StencilEnable = TRUE;
      SetDepthState(depth);
      ctx->Draw(3, 3);

      // 3: write background
      depth.StencilEnable = FALSE;
      SetDepthState(depth);
      ctx->Draw(3, 6);

      RSSetViewport(
          {10.0f, 10.0f, (float)screenWidth - 20.0f, (float)screenHeight - 20.0f, 0.0f, 1.0f});

      // add a marker so we can easily locate this draw
      annot->BeginEvent(L"Test Begin");

      depth.StencilEnable = TRUE;
      depth.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER;
      SetDepthState(depth);
      ctx->Draw(21, 9);

      annot->EndEvent();

      Present();
    }

    return 0;
  }
};

REGISTER_TEST(D3D11_Overlay_Test);