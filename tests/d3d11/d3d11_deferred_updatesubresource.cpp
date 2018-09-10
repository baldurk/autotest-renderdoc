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

Texture2D<float4> tex;

float4 main(v2f IN) : SV_Target0
{
	clip(float2(1.0f, 1.0f) - IN.uv.xy);
	return tex.Load(int3(IN.uv.xyz*64.0f));
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  // the main tex in this test
  ID3D11Texture2DPtr tex, tex2;
  ID3D11ShaderResourceViewPtr srv, srv2;

  ID3D11DeviceContextPtr defctx;
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  dev->CreateDeferredContext(0, &defctx);

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

  a2v triangle[] = {
      {
          Vec3f(-0.5f, 0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(-0.5f, 1.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 2.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(2.0f, 0.0f),
      },

      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 1.0f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 2.0f),
      },
      {
          Vec3f(0.5f, 0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(2.0f, 0.0f),
      },

      {
          Vec3f(-0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(-0.5f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 2.0f),
      },
      {
          Vec3f(0.0f, 0.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(2.0f, 0.0f),
      },

      {
          Vec3f(0.0f, 0.0f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 2.0f),
      },
      {
          Vec3f(0.5f, 0.0f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(2.0f, 0.0f),
      },
  };

  if(MakeBuffer(eVBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle, &vb, NULL, NULL,
                NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  if(MakeTexture2D(64, 64, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &tex, &srv, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create texture");
    return 1;
  }

  if(MakeTexture2D(2048, 2048, 1, DXGI_FORMAT_R8_UNORM, &tex2, &srv2, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create texture");
    return 1;
  }

  float *buffers[3];

  // each buffer is twice the size of the texture so we can see any reads from
  // before the source area
  for(size_t i = 0; i < 3; i++)
    buffers[i] = new float[64 * 64 * 4 * 2];

  for(size_t i = 0; i < 64 * 64 * 2; i++)
  {
    // first buffer is dark grey
    buffers[0][i * 4 + 0] = 0.1f;
    buffers[0][i * 4 + 1] = 0.1f;
    buffers[0][i * 4 + 2] = 0.1f;
    buffers[0][i * 4 + 3] = 1.0f;

    // others have red to mark 'incorrect' areas.
    // should never be read from
    for(size_t x = 1; x < 3; x++)
    {
      buffers[x][i * 4 + 0] = 1.0f;
      buffers[x][i * 4 + 1] = 0.0f;
      buffers[x][i * 4 + 2] = 0.0f;
      buffers[x][i * 4 + 3] = 1.0f;
    }
  }

  float *srcArea[4];
  for(size_t i = 0; i < 3; i++)
    srcArea[i] = buffers[i] + 64 * 64 * 4;

  for(size_t i = 0; i < 16 * 16; i++)
  {
    // fill first buffer with random green colours
    srcArea[1][i * 4 + 0] = 0.2f;
    srcArea[1][i * 4 + 1] = RANDF(0.0f, 1.0f);
    srcArea[1][i * 4 + 2] = 0.2f;
    srcArea[1][i * 4 + 3] = 1.0f;

    // second with random blue colours
    srcArea[2][i * 4 + 0] = 0.2f;
    srcArea[2][i * 4 + 1] = 0.2f;
    srcArea[2][i * 4 + 2] = RANDF(0.0f, 1.0f);
    srcArea[2][i * 4 + 3] = 1.0f;
  }

  D3D11_BOX leftBox = CD3D11_BOX(4, 4, 0, 20, 20, 1);
  D3D11_BOX toprightBox = CD3D11_BOX(44, 44, 0, 60, 60, 1);
  D3D11_BOX botrightBox = CD3D11_BOX(44, 4, 0, 60, 20, 1);

  // corrected deferred context version of srcArea[2]
  srcArea[3] = srcArea[2];
  // pAdjustedSrcData = ((const BYTE*)pSrcData) - (alignedBox.front * srcDepthPitch) -
  // (alignedBox.top * srcRowPitch) - (alignedBox.left * srcBytesPerElement);

  D3D11_FEATURE_DATA_THREADING threadingCaps = {FALSE, FALSE};

  hr = dev->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));
  if(SUCCEEDED(hr))
  {
    if(!threadingCaps.DriverCommandLists)
    {
      srcArea[3] = (float *)(((BYTE *)srcArea[3]) - (botrightBox.top * 16 * sizeof(float) * 4) -
                             (botrightBox.left * sizeof(float) * 4));
    }
  }

  ID3D11CommandListPtr cmdList;

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    UINT stride = sizeof(a2v);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(layout);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);

    // first clear the texture with no box, fully black, on the immediate context
    ctx->UpdateSubresource(tex, 0, NULL, srcArea[0], 64 * sizeof(float) * 4,
                           64 * 64 * sizeof(float) * 4);

    ctx->Draw(3, 0);

    // now write some random green bits into a left box, on the immediate context
    ctx->UpdateSubresource(tex, 0, &leftBox, srcArea[1], 16 * sizeof(float) * 4,
                           16 * 16 * sizeof(float) * 4);

    ctx->Draw(3, 3);

    // now write some random blue bits into a left box, on the deferred context, WITHOUT correction
    defctx->UpdateSubresource(tex, 0, &toprightBox, srcArea[2], 16 * sizeof(float) * 4,
                              16 * 16 * sizeof(float) * 4);

    defctx->FinishCommandList(TRUE, &cmdList);

    ctx->ExecuteCommandList(cmdList, TRUE);

    ctx->Draw(3, 6);

    cmdList = NULL;

    // now write some random blue bits into a left box, on the deferred context, WITH correction
    defctx->UpdateSubresource(tex, 0, &botrightBox, srcArea[3], 16 * sizeof(float) * 4,
                              16 * 16 * sizeof(float) * 4);

    defctx->FinishCommandList(TRUE, &cmdList);

    ctx->ExecuteCommandList(cmdList, TRUE);

    ctx->Draw(3, 9);

    ID3D11Device *device = dev;
    ID3D11DeviceContext *context = ctx;

    // test update with box to ensure we don't read too much data
    D3D11_BOX smallbox = CD3D11_BOX(2000, 2000, 0, 2040, 2040, 1);
    byte *smalldata = new byte[2048 * 39 + 40];
    memset(smalldata, 0xfd, 2048 * 39 + 40);
    ctx->UpdateSubresource(tex2, 0, &smallbox, smalldata, 2048, 0);

    delete[] smalldata;

    cmdList = NULL;

    Present();
  }

  for(size_t i = 0; i < 3; i++)
    delete[] buffers[i];

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Deferred_UpdateSubresource",
              "Test that does UpdateSubresource on a deferred context which might need some "
              "workaround code.");