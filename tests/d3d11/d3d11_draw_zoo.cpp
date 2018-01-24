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

  float vertidx : VID;
  float instidx : IID;
};

)EOSHADER";

string vertex = R"EOSHADER(

v2f main(a2v IN, uint vid : SV_VertexID, uint instid : SV_InstanceID)
{
	v2f OUT = (v2f)0;

	OUT.pos = float4(IN.pos.xyz, 1);
  OUT.pos.x += 0.1f*instid;
	OUT.col = IN.col;
	OUT.uv = float4(IN.uv, 0, 1);

  OUT.vertidx = float(vid);
  OUT.instidx = float(instid);

	return OUT;
}

)EOSHADER";

string pixel = R"EOSHADER(

float4 main(v2f IN) : SV_Target0
{
	float4 ret = IN.col;
  ret.g = IN.vertidx/30.0f;
  ret.b = IN.instidx/10.0f;
  return ret;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout, instlayout;
  ID3D11BufferPtr vb, instvb, ib;

  ID3D11RasterizerStatePtr rs;

  ID3D11VertexShaderPtr vs;
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

  layoutdesc[1].InputSlot = 1;
  layoutdesc[1].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
  layoutdesc[1].InstanceDataStepRate = 1;

  CHECK_HR(dev->CreateInputLayout(layoutdesc, ARRAY_COUNT(layoutdesc), vsblob->GetBufferPointer(),
                                  vsblob->GetBufferSize(), &instlayout));

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  a2v triangle[] = {
      // 0
      {
          Vec3f(-1.0f, -1.0f, -1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(-1.0f, -1.0f),
      },
      // 1, 2, 3
      {
          Vec3f(-0.5f, 0.5f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 4, 5, 6
      {
          Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 7, 8, 9
      {
          Vec3f(-0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 10, 11, 12
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  std::vector<a2v> vbData;
  vbData.resize(30);

  {
    a2v *src = (a2v *)triangle;
    a2v *dst = (a2v *)&vbData[0];

    // up-pointing triangle to offset 0
    memcpy(dst + 0, triangle + 1, sizeof(a2v));
    memcpy(dst + 1, triangle + 2, sizeof(a2v));
    memcpy(dst + 2, triangle + 3, sizeof(a2v));

    // invalid vert for index 3 and 4
    memcpy(dst + 3, triangle + 0, sizeof(a2v));
    memcpy(dst + 4, triangle + 0, sizeof(a2v));

    // down-pointing triangle at offset 5
    memcpy(dst + 5, triangle + 4, sizeof(a2v));
    memcpy(dst + 6, triangle + 5, sizeof(a2v));
    memcpy(dst + 7, triangle + 6, sizeof(a2v));

    // invalid vert for 8 - 12
    memcpy(dst + 8, triangle + 0, sizeof(a2v));
    memcpy(dst + 9, triangle + 0, sizeof(a2v));
    memcpy(dst + 10, triangle + 0, sizeof(a2v));
    memcpy(dst + 11, triangle + 0, sizeof(a2v));
    memcpy(dst + 12, triangle + 0, sizeof(a2v));

    // left-pointing triangle data to offset 13
    memcpy(dst + 13, triangle + 7, sizeof(a2v));
    memcpy(dst + 14, triangle + 8, sizeof(a2v));
    memcpy(dst + 15, triangle + 9, sizeof(a2v));

    // invalid vert for 16-22
    memcpy(dst + 16, triangle + 0, sizeof(a2v));
    memcpy(dst + 17, triangle + 0, sizeof(a2v));
    memcpy(dst + 18, triangle + 0, sizeof(a2v));
    memcpy(dst + 19, triangle + 0, sizeof(a2v));
    memcpy(dst + 20, triangle + 0, sizeof(a2v));
    memcpy(dst + 21, triangle + 0, sizeof(a2v));
    memcpy(dst + 22, triangle + 0, sizeof(a2v));

    // right-pointing triangle data to offset 23
    memcpy(dst + 23, triangle + 10, sizeof(a2v));
    memcpy(dst + 24, triangle + 11, sizeof(a2v));
    memcpy(dst + 25, triangle + 12, sizeof(a2v));
  }

  if(MakeBuffer(eVBuffer, 0, UINT(vbData.size() * sizeof(a2v)), 0, DXGI_FORMAT_UNKNOWN,
                vbData.data(), &vb, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  Vec4f instData[16] = {};

  {
    instData[0] = Vec4f(0.0f, 0.5f, 1.0f, 1.0f);
    instData[1] = Vec4f(1.0f, 0.5f, 0.0f, 1.0f);

    instData[5] = Vec4f(1.0f, 0.5f, 0.5f, 1.0f);
    instData[6] = Vec4f(0.5f, 0.5f, 1.0f, 1.0f);

    instData[13] = Vec4f(0.1f, 0.8f, 0.3f, 1.0f);
    instData[14] = Vec4f(0.8f, 0.1f, 0.1f, 1.0f);
  }

  if(MakeBuffer(eVBuffer, 0, sizeof(instData), 0, DXGI_FORMAT_UNKNOWN, instData, &instvb, NULL,
                NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle inst VB");
    return 1;
  }

  std::vector<uint32_t> idxData;
  idxData.resize(50);

  {
    idxData[0] = 0;
    idxData[1] = 1;
    idxData[2] = 2;

    idxData[5] = 5;
    idxData[6] = 6;
    idxData[7] = 7;

    idxData[13] = 63;
    idxData[14] = 64;
    idxData[15] = 65;

    idxData[23] = 103;
    idxData[24] = 104;
    idxData[25] = 105;

    idxData[37] = 104;
    idxData[38] = 105;
    idxData[39] = 106;
  }

  if(MakeBuffer(eIBuffer, 0, UINT(idxData.size() * sizeof(uint32_t)), 0, DXGI_FORMAT_UNKNOWN,
                idxData.data(), &ib, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle inst VB");
    return 1;
  }

  CD3D11_RASTERIZER_DESC rd = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
  rd.CullMode = D3D11_CULL_NONE;

  CHECK_HR(dev->CreateRasterizerState(&rd, &rs));

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->RSSetState(rs);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);

    D3D11_VIEWPORT view = {0.0f, 0.0f, 128.0f, 128.0f, 0.0f, 1.0f};

    ctx->RSSetViewports(1, &view);

    ID3D11Buffer *vbs[2] = {vb, instvb};
    UINT strides[2] = {sizeof(a2v), sizeof(Vec4f)};
    UINT offsets[2] = {0, 0};

    ctx->IASetInputLayout(layout);

    ///////////////////////////////////////////////////
    // non-indexed, non-instanced

    // basic test
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->Draw(3, 0);
    view.TopLeftX += view.Width;

    // test with vertex offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->Draw(3, 5);
    view.TopLeftX += view.Width;

    // test with vertex offset and vbuffer offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 5 * sizeof(a2v);
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->Draw(3, 8);
    view.TopLeftX += view.Width;

    // adjust to next row
    view.TopLeftX = 0.0f;
    view.TopLeftY += view.Height;

    ///////////////////////////////////////////////////
    // indexed, non-instanced

    // basic test
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexed(3, 0, 0);
    view.TopLeftX += view.Width;

    // test with first index
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexed(3, 5, 0);
    view.TopLeftX += view.Width;

    // test with first index and vertex offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexed(3, 13, -50);
    view.TopLeftX += view.Width;

    // test with first index and vertex offset and vbuffer offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 10 * sizeof(a2v);
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexed(3, 23, -100);
    view.TopLeftX += view.Width;

    // test with first index and vertex offset and vbuffer offset and ibuffer offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 19 * sizeof(a2v);
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 14 * sizeof(uint32_t));
    ctx->DrawIndexed(3, 23, -100);
    view.TopLeftX += view.Width;

    // adjust to next row
    view.TopLeftX = 0.0f;
    view.TopLeftY += view.Height;

    ctx->IASetInputLayout(instlayout);

    ///////////////////////////////////////////////////
    // non-indexed, instanced

    // basic test
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    offsets[1] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->DrawInstanced(3, 2, 0, 0);
    view.TopLeftX += view.Width;

    // basic test with first instance
    ctx->RSSetViewports(1, &view);
    offsets[0] = 5 * sizeof(a2v);
    offsets[1] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->DrawInstanced(3, 2, 0, 5);
    view.TopLeftX += view.Width;

    // basic test with first instance and instance buffer offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 13 * sizeof(a2v);
    offsets[1] = 8 * sizeof(Vec4f);
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->DrawInstanced(3, 2, 0, 5);
    view.TopLeftX += view.Width;

    // adjust to next row
    view.TopLeftX = 0.0f;
    view.TopLeftY += view.Height;

    ///////////////////////////////////////////////////
    // indexed, instanced

    // basic test
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    offsets[1] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexedInstanced(3, 2, 5, 0, 0);
    view.TopLeftX += view.Width;

    // basic test with first instance
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    offsets[1] = 0;
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexedInstanced(3, 2, 13, -50, 5);
    view.TopLeftX += view.Width;

    // basic test with first instance and instance buffer offset
    ctx->RSSetViewports(1, &view);
    offsets[0] = 0;
    offsets[1] = 8 * sizeof(Vec4f);
    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->DrawIndexedInstanced(3, 2, 23, -80, 5);
    view.TopLeftX += view.Width;

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Draw_Zoo", "Draws several variants using different vertex/index offsets.");