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

  ID3D11Texture2DPtr rt;
  ID3D11RenderTargetViewPtr rtv[4];
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = Compile(vertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = Compile(pixel, "main", "ps_5_0");

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  MakeTexture2D(1024, 1024, 8, DXGI_FORMAT_R8G8B8A8_UNORM, &rt, NULL, NULL, &rtv[0], NULL);

  CD3D11_RENDER_TARGET_VIEW_DESC desc1(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
  CD3D11_RENDER_TARGET_VIEW_DESC desc2(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 2);
  CD3D11_RENDER_TARGET_VIEW_DESC desc3(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 3);
  CHECK_HR(dev->CreateRenderTargetView(rt, &desc1, &rtv[1]));
  CHECK_HR(dev->CreateRenderTargetView(rt, &desc2, &rtv[2]));
  CHECK_HR(dev->CreateRenderTargetView(rt, &desc3, &rtv[3]));

  Vec4f col;

  if(MakeBuffer(eCBuffer, 0, sizeof(Vec4f), 0, DXGI_FORMAT_UNKNOWN, &col, &cb, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create CB");
    return 1;
  }

  D3D11_VIEWPORT view0 = {0.0f, 0.0f, (float)1024.0f, (float)1024.0f, 0.0f, 1.0f};
  D3D11_VIEWPORT view1 = {0.0f, 0.0f, (float)512.0f, (float)512.0f, 0.0f, 1.0f};
  D3D11_VIEWPORT view2 = {0.0f, 0.0f, (float)256.0f, (float)256.0f, 0.0f, 1.0f};
  D3D11_VIEWPORT view3 = {0.0f, 0.0f, (float)128.0f, (float)128.0f, 0.0f, 1.0f};

  while(Running())
  {
    float clearcol[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, clearcol);
    for(int i = 0; i < 4; i++)
      ctx->ClearRenderTargetView(rtv[i], clearcol);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->PSSetConstantBuffers(0, 1, &cb.GetInterfacePtr());

    col = Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));
    ctx->RSSetViewports(1, &view0);
    ctx->OMSetRenderTargets(1, &rtv[0].GetInterfacePtr(), NULL);

    ctx->Draw(4, 0);

    col = Vec4f(0.0f, 1.0f, 0.0f, 1.0f);
    ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));
    ctx->RSSetViewports(1, &view1);
    ctx->OMSetRenderTargets(1, &rtv[1].GetInterfacePtr(), NULL);

    ctx->Draw(4, 0);

    col = Vec4f(0.0f, 0.0f, 1.0f, 1.0f);
    ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));
    ctx->RSSetViewports(1, &view2);
    ctx->OMSetRenderTargets(1, &rtv[2].GetInterfacePtr(), NULL);

    ctx->Draw(4, 0);

    col = Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
    ctx->UpdateSubresource(cb, 0, NULL, &col, sizeof(col), sizeof(col));
    ctx->RSSetViewports(1, &view3);
    ctx->OMSetRenderTargets(1, &rtv[3].GetInterfacePtr(), NULL);

    ctx->Draw(4, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int D3D11_Mip_RTV(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}
