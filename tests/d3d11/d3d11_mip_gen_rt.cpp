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
string common = R"EOSHADER(

struct v2f
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
};

)EOSHADER";

string vertex = R"EOSHADER(

v2f main(uint vid : SV_VertexID)
{
	v2f OUT = (v2f)0;

	float2 positions[] = {
		float2(-1.0f,  1.0f),
		float2( 1.0f,  1.0f),
		float2(-1.0f, -1.0f),
		float2( 1.0f, -1.0f),
	};

	OUT.pos = float4(positions[vid], 0, 1);
	OUT.uv = positions[vid]*float2(1,-1)*0.5f + 0.5f;

	return OUT;
}

)EOSHADER";

string pixel = R"EOSHADER(

Texture2D<float4> intex : register(t0);
SamplerState s : register(s0);

float4 main(v2f IN) : SV_Target0
{
	return intex.Sample(s, IN.uv);
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11SamplerStatePtr samp;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  static const int NumMips = 8;

  ID3D11Texture2DPtr rt;
  ID3D11RenderTargetViewPtr rtv[NumMips];
  ID3D11ShaderResourceViewPtr srv[NumMips];
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = Compile(common + vertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = Compile(common + pixel, "main", "ps_5_0");

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  MakeTexture2D(1024, 1024, NumMips, DXGI_FORMAT_R8G8B8A8_UNORM, &rt,
                (ID3D11ShaderResourceView **)0x1, NULL, &rtv[0], NULL);

  CD3D11_SAMPLER_DESC sampdesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
  CHECK_HR(dev->CreateSamplerState(&sampdesc, &samp));

  D3D11_VIEWPORT views[NumMips];

  for(int i = 0; i < NumMips; i++)
  {
    CD3D11_RENDER_TARGET_VIEW_DESC rdesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM,
                                         i);
    CHECK_HR(dev->CreateRenderTargetView(rt, &rdesc, &rtv[i]));

    CD3D11_SHADER_RESOURCE_VIEW_DESC sdesc(D3D11_SRV_DIMENSION_TEXTURE2D,
                                           DXGI_FORMAT_R8G8B8A8_UNORM, i, 1);
    CHECK_HR(dev->CreateShaderResourceView(rt, &sdesc, &srv[i]));

    views[i] = CD3D11_VIEWPORT(0.0f, 0.0f, (float)(512 >> i), (float)(512 >> i));
  }

  // fill upper mip with colour ramp
  uint32_t *ramp = new uint32_t[1024 * 1024];
  for(uint32_t i = 0; i < 1024 * 1024; i++)
  {
    float x = float(i % 1024);
    float y = float(i / 1024);
    ramp[i] = uint32_t(uint32_t(255.0f * (x / 1024.0f)) | (uint32_t(255.0f * (y / 1024.0f)) << 8) |
                       (uint32_t(255.0f * ((x + y) / 2048.0f)) << 16) | 0xff000000);
  }

  while(Running())
  {
    float clearcol[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, clearcol);
    for(int i = 0; i < NumMips; i++)
      ctx->ClearRenderTargetView(rtv[i], clearcol);

    ctx->UpdateSubresource(rt, 0, NULL, ramp, 1024 * sizeof(uint32_t),
                           1024 * 1024 * sizeof(uint32_t));

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->PSSetSamplers(0, 1, &samp.GetInterfacePtr());

    // downsample chain
    for(int i = 0; i < NumMips - 1; i++)
    {
      ctx->RSSetViewports(1, &views[i]);
      ctx->OMSetRenderTargets(1, &rtv[i + 1].GetInterfacePtr(), NULL);
      ctx->PSSetShaderResources(0, 1, &srv[i].GetInterfacePtr());
      ctx->Draw(4, 0);
    }

    // now test that 'invalid' binds still get detected
    ctx->OMSetRenderTargets(1, &rtv[0].GetInterfacePtr(), NULL);
    ctx->PSSetShaderResources(0, 1, &srv[0].GetInterfacePtr());    // should bind NULL

    ctx->PSSetShaderResources(0, 1, &srv[1].GetInterfacePtr());
    ctx->OMSetRenderTargets(1, &rtv[1].GetInterfacePtr(),
                            NULL);    // should cause SRV to be unbound

    Present();
  }

  delete[] ramp;

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Mip_Gen_RT",
              "Tests rendering from one mip to another to do a downsample chain");