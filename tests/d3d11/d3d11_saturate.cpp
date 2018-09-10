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

#include "../d3d11_common.h"

struct Saturate : D3D11GraphicsTest
{
  static constexpr char *Description = "Tests using saturate, originally for a bug report";

  string vertex = R"EOSHADER(

void main(uint vert : SV_VertexID, out float4 pos : SV_Position)
{
  float2 positions[] = {
    float2(0.0f,  0.0f),
    float2(1.0f,  0.0f),
    float2(0.0f, -1.0f),
  };

  pos = float4(positions[vert], 0, 1);
}

)EOSHADER";

  string pixel = R"EOSHADER(

void main(float4 pos : SV_Position, out float4 a : SV_Target0, out float4 b : SV_Target1)
{
  // this code is arbitrary, just to get a negative value and ensure
  // it's a) not known ahead of time at all
  // b) not merged in with any of the calculations pre-saturate below
  float negative = log2(pos.x / 1000.0f);

  // maps to mov_sat
  float zero = saturate(negative);
  // maps to add_sat which breaks
  float addsatzero = saturate(negative - 1.00001f);
  // maps to mul_sat
  float mulsatzero = saturate(negative * 1.00001f);

  a.x = negative;
  a.y = zero;
  a.z = addsatzero;
  a.w = mulsatzero;

  b.x = float(zero == 0.0f);
  b.y = float(addsatzero == 0.0f);
  b.z = float(mulsatzero == 0.0f);
  b.w = 0.0f;
}

)EOSHADER";

  int main(int argc, char **argv)
  {
    // initialise, create window, create device, etc
    if(!Init(argc, argv))
      return 3;

    HRESULT hr = S_OK;

    ID3DBlobPtr vsblob = Compile(vertex, "main", "vs_5_0");
    ID3DBlobPtr psblob = Compile(pixel, "main", "ps_5_0");

    ID3D11VertexShaderPtr vs = CreateVS(vsblob);
    ID3D11PixelShaderPtr ps = CreatePS(psblob);

    ID3D11Texture2DPtr fltTex[2];
    ID3D11RenderTargetViewPtr fltRT[2];
    MakeTexture2D(400, 400, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &fltTex[0], NULL, NULL, &fltRT[0],
                  NULL);
    MakeTexture2D(400, 400, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &fltTex[1], NULL, NULL, &fltRT[1],
                  NULL);

    while(Running())
    {
      float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
      ctx->ClearRenderTargetView(fltRT[0], col);
      ctx->ClearRenderTargetView(fltRT[1], col);
      ctx->ClearRenderTargetView(bbRTV, col);

      ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      ctx->VSSetShader(vs, NULL, 0);
      ctx->PSSetShader(ps, NULL, 0);

      D3D11_VIEWPORT view = {0.0f, 0.0f, 400.0f, 400.0f, 0.0f, 1.0f};
      ctx->RSSetViewports(1, &view);

      ID3D11RenderTargetView *rts[] = {
          fltRT[0], fltRT[1],
      };
      ctx->OMSetRenderTargets(2, rts, NULL);

      ctx->Draw(3, 0);

      Present();
    }

    return 0;
  }
};

REGISTER_TEST(Saturate);