/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Baldur Karlsson
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
};

)EOSHADER";

string vertex = R"EOSHADER(

v2f main(a2v IN, uint vid : SV_VertexID)
{
	v2f OUT = (v2f)0;

	OUT.pos = float4(IN.pos.xyz, 1);
	OUT.col = IN.col;

	return OUT;
}

)EOSHADER";

string pixel = R"EOSHADER(

float4 main(v2f IN) : SV_Target0
{
	return IN.col;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

  ID3D11BufferPtr so[2];

  ID3D11VertexShaderPtr vs;
  ID3D11GeometryShaderPtr gs;
  ID3D11PixelShaderPtr ps;
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

  std::vector<D3D11_SO_DECLARATION_ENTRY> sodecl;
  std::vector<UINT> strides;

  {
    D3D11_SO_DECLARATION_ENTRY decl;
    decl.StartComponent = 0;
    decl.ComponentCount = 4;
    decl.SemanticName = "SV_POSITION";
    decl.SemanticIndex = 0;

    decl.Stream = 0;
    decl.OutputSlot = 0;
    sodecl.push_back(decl);
  }
  {
    D3D11_SO_DECLARATION_ENTRY decl;
    decl.StartComponent = 0;
    decl.ComponentCount = 4;
    decl.SemanticName = "COLOR";
    decl.SemanticIndex = 0;

    decl.Stream = 0;
    decl.OutputSlot = 1;
    sodecl.push_back(decl);
  }
  strides.push_back(4 * sizeof(float));
  strides.push_back(8 * sizeof(float));

  ID3D11ShaderReflection *foo = NULL;
  D3DReflect(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), __uuidof(ID3D11ShaderReflection),
             (void **)&foo);

  D3D11_SIGNATURE_PARAMETER_DESC outdesc0;
  foo->GetOutputParameterDesc(0, &outdesc0);

  D3D11_SIGNATURE_PARAMETER_DESC outdesc1;
  foo->GetOutputParameterDesc(1, &outdesc1);

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreateGeometryShaderWithStreamOutput(
      vsblob->GetBufferPointer(), vsblob->GetBufferSize(), &sodecl[0], (UINT)sodecl.size(),
      &strides[0], (UINT)strides.size(), 0, NULL, &gs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  a2v triangle[] = {
      {
          Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  if(MakeBuffer(eVBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle, &vb, NULL, NULL,
                NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  if(MakeBuffer(eSOBuffer, 0, 2048, 0, DXGI_FORMAT_UNKNOWN, triangle, &so[0], NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  if(MakeBuffer(eSOBuffer, 0, 2048, 0, DXGI_FORMAT_UNKNOWN, triangle, &so[1], NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  while(Running())
  {
    ctx->ClearState();

    unsigned char empty[2048] = {};
    ctx->UpdateSubresource(so[0], 0, NULL, empty, 2048, 2048);
    ctx->UpdateSubresource(so[1], 0, NULL, empty, 2048, 2048);

    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    UINT stride = sizeof(a2v);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(layout);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->GSSetShader(gs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);

    ID3D11Buffer *bufs[] = {so[0], so[1]};
    UINT offs[2] = {0};
    ctx->SOSetTargets(2, bufs, offs);

    ctx->Draw(3, 0);

    ctx->UpdateSubresource(so[0], 0, NULL, empty, 2048, 2048);
    ctx->UpdateSubresource(so[1], 0, NULL, empty, 2048, 2048);

    ctx->ClearRenderTargetView(bbRTV, col);

    ctx->Draw(3, 0);

    ctx->UpdateSubresource(so[0], 0, NULL, empty, 2048, 2048);
    ctx->UpdateSubresource(so[1], 0, NULL, empty, 2048, 2048);

    ctx->ClearRenderTargetView(bbRTV, col);

    // test using offsets of NULL. Should be equivalent to passing -1
    bufs[0] = so[1];
    bufs[1] = so[0];
    ctx->SOSetTargets(2, bufs, NULL);

    ctx->Draw(3, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int D3D11_StreamOut(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}