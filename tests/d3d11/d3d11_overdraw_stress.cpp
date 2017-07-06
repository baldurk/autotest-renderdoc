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
struct a2v
{
  Vec3f pos;
  Vec4f col;
  Vec2f uv;
};

string common = R"EOSHADER(

struct a2v
{
	float3 pos : POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

struct v2f
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float4 uv : TEXCOORD0;
};

)EOSHADER";

string vertex = R"EOSHADER(

v2f main(a2v IN, uint vid : SV_VertexID)
{
	v2f OUT = (v2f)0;

	OUT.pos = float4(IN.pos.xyz, 1);
	OUT.col = IN.col;
	OUT.uv = float4(IN.uv, 0, 1);

	return OUT;
}

)EOSHADER";

string pixel = R"EOSHADER(

float4 main(v2f IN, bool fface : SV_IsFrontFace) : SV_Target0
{
	return fface ? IN.col : float4(1, 0, 0, 1);
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11RasterizerStatePtr rs;
  ID3D11DepthStencilStatePtr ds;
  ID3D11BlendStatePtr bs;

  ID3D11Texture2DPtr bbDepth;
  ID3D11DepthStencilViewPtr bbDSV;
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = Compile(common + vertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = Compile(common + pixel, "main", "ps_5_0");

  D3D11_INPUT_ELEMENT_DESC layoutdesc[] = {
      {
          "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
      {
          "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
          D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
      {
          "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
          D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
  };

  CHECK_HR(dev->CreateInputLayout(layoutdesc, ARRAY_COUNT(layoutdesc), vsblob->GetBufferPointer(),
                                  vsblob->GetBufferSize(), &layout));

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  MakeTexture2D(screenWidth, screenHeight, 1, DXGI_FORMAT_D32_FLOAT_S8X24_UINT, &bbDepth, NULL,
                NULL, NULL, &bbDSV);

  CD3D11_RASTERIZER_DESC rd = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
  rd.CullMode = D3D11_CULL_NONE;

  CHECK_HR(dev->CreateRasterizerState(&rd, &rs));

  CD3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
  bd.IndependentBlendEnable = TRUE;
  bd.RenderTarget[0].BlendEnable = TRUE;
  bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
  bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
  bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;
  bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;
  bd.RenderTarget[0].RenderTargetWriteMask = 0xf;

  CHECK_HR(dev->CreateBlendState(&bd, &bs));

  CD3D11_DEPTH_STENCIL_DESC dd = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
  dd.DepthEnable = FALSE;
  dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dd.DepthFunc = D3D11_COMPARISON_LESS;
  dd.StencilEnable = FALSE;
  dd.StencilWriteMask = dd.StencilReadMask = 0xff;

  CHECK_HR(dev->CreateDepthStencilState(&dd, &ds));

  srand(0U);

  const size_t numVerts = 1200;

  a2v triangle[numVerts] = {0};

  for(int i = 0; i < numVerts; i++)
  {
    triangle[i].pos.x = ((float(rand()) / float(RAND_MAX)) - 0.5f) * 2.0f;
    triangle[i].pos.y = ((float(rand()) / float(RAND_MAX)) - 0.5f) * 2.0f;
    triangle[i].pos.z = ((float(rand()) / float(RAND_MAX)) - 0.5f) * 2.0f;

    triangle[i].col.x = float(rand()) / float(RAND_MAX);
    triangle[i].col.y = float(rand()) / float(RAND_MAX);
    triangle[i].col.z = float(rand()) / float(RAND_MAX);
  }

  if(MakeBuffer(eVBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle, &vb, NULL, NULL,
                NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    ctx->ClearDepthStencilView(bbDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    UINT stride = sizeof(a2v);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(layout);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);
    ctx->RSSetState(rs);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);
    float bf[] = {1.0f, 0.0f, 1.0f, 0.0f};
    ctx->OMSetBlendState(bs, bf, ~0U);
    ctx->OMSetDepthStencilState(ds, 0);

    ctx->Draw(numVerts, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int D3D11_Overdraw_Stress(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}
