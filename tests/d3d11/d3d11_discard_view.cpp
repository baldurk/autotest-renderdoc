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

namespace
{
string vertex = R"EOSHADER(

float4 main(uint vid : SV_VertexID) : SV_POSITION
{
	float2 positions[] = {
		float2(-1.0f,  1.0f),
		float2( 1.0f,  1.0f),
		float2(-1.0f, -1.0f),
		float2( 1.0f, -1.0f),
	};

	return float4(positions[vid], 0, 1);
}

)EOSHADER";

string pixel = R"EOSHADER(

cbuffer consts : register(b0)
{
	float4 col;
};

float4 main() : SV_Target0
{
	return col;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11BufferPtr cb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11Texture2DPtr tex_rt;
};

int impl::main(int argc, char **argv)
{
  d3d11_1 = true;

  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = Compile(vertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = Compile(pixel, "main", "ps_5_0");

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  MakeTexture2D(screenWidth, screenHeight, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &tex_rt, NULL, NULL,
                (ID3D11RenderTargetView **)0x1, NULL);

  ID3D11RenderTargetViewPtr rtv;
  CD3D11_RENDER_TARGET_VIEW_DESC desc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

  Vec4f col;

  if(MakeBuffer(eCBuffer, 0, sizeof(Vec4f), 0, DXGI_FORMAT_UNKNOWN, &col, &cb, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create CB");
    return 1;
  }

  D3D11_VIEWPORT view[10];
  for(int i = 0; i < 10; i++)
  {
    view[i].MinDepth = 0.0f;
    view[i].MaxDepth = 1.0f;
    view[i].TopLeftX = (float)i * 50.0f;
    view[i].TopLeftY = 0.0f;
    view[i].Width = 50.0f;
    view[i].Height = 250.0f;
  }

  D3D11_VIEWPORT fullview;
  {
    fullview.MinDepth = 0.0f;
    fullview.MaxDepth = 1.0f;
    fullview.TopLeftX = 0.0f;
    fullview.TopLeftY = 0.0f;
    fullview.Width = (float)screenWidth;
    fullview.Height = (float)screenHeight;
  }

  while(Running())
  {
    ctx1->DiscardView(bbRTV);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->PSSetConstantBuffers(0, 1, &cb.GetInterfacePtr());

    {
      ctx->RSSetViewports(1, &fullview);

      rtv = NULL;
      CHECK_HR(dev->CreateRenderTargetView(tex_rt, &desc, &rtv));

      ctx->OMSetRenderTargets(1, &rtv.GetInterfacePtr(), NULL);

      col = Vec4f(RANDF(0.0f, 1.0f), RANDF(0.0f, 1.0f), RANDF(0.0f, 1.0f), 1.0f);
      ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));

      ctx->Draw(4, 0);
    }

    for(int i = 0; i < 10; i++)
    {
      ctx->RSSetViewports(1, view + i);

      rtv = NULL;
      CHECK_HR(dev->CreateRenderTargetView(tex_rt, &desc, &rtv));

      ctx->OMSetRenderTargets(1, &rtv.GetInterfacePtr(), NULL);

      col = Vec4f(RANDF(0.0f, 1.0f), RANDF(0.0f, 1.0f), RANDF(0.0f, 1.0f), 1.0f);
      ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));

      ctx->Draw(4, 0);
    }

    ctx->CopyResource(bbTex, tex_rt);

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Discard_View", "Test that discards an RTV");