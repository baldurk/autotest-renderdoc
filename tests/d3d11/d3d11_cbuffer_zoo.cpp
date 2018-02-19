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

cbuffer consts : register(b0)
{
  // dummy* entries are just to 'reset' packing to avoid pollution between tests

  float4 a;                               // basic float4 = {0, 1, 2, 3}
  float3 b;                               // should have a padding word at the end = {4, 5, 6}, <7>

  float2 c; float2 d;                     // should be packed together = {8, 9}, {10, 11}
  float e; float3 f;                      // should be packed together = 12, {13, 14, 15}
  float g; float2 h; float i;             // should be packed together = 16, {17, 18}, 19
  float j; float2 k;                      // should have a padding word at the end = 20, {21, 22}, <23>
  float2 l; float m;                      // should have a padding word at the end = {24, 25}, 26, <27>

  float n[4];                             // should cover 4 float4s = 28, <29..31>, 32, <33..35>, 36, <37..39>, 40
  float4 dummy1;

  float o[4];                             // should cover 4 float4s = 48, <..>, 52, <..>, 56, <..>, 60
  float p;                                // should be packed in with above array, with two padding words = 61, <62, 63>
  float4 dummy2;

  float4 dummygl1;                         // padding to match GL so matrices start on same values
  float4 dummygl2;

  column_major float4x4 q;                // should cover 4 float4s.
                                          // row0: {76, 80, 84, 88}
                                          // row1: {77, 81, 85, 89}
                                          // row2: {78, 82, 86, 90}
                                          // row3: {79, 83, 87, 91}
  row_major float4x4 r;                   // should cover 4 float4s
                                          // row0: {92, 93, 94, 95}
                                          // row1: {96, 97, 98, 99}
                                          // row2: {100, 101, 102, 103}
                                          // row3: {104, 105, 106, 107}

  column_major float3x4 s;                // covers 4 float4s with padding at end of each column
                                          // row0: {108, 112, 116, 120}
                                          // row1: {109, 113, 117, 121}
                                          // row2: {110, 114, 118, 122}
                                          //       <111, 115, 119, 123>
  float4 dummy3;
  row_major float3x4 t;                   // covers 3 float4s with no padding
                                          // row0: {128, 129, 130, 131}
                                          // row1: {132, 133, 134, 135}
                                          // row2: {136, 137, 138, 139}
  float4 dummy4;

  column_major float2x3 u;                // covers 3 float4s with padding at end of each column (but not row)
                                          // row0: {144, 148, 152}
                                          // row1: {145, 149, 153}
                                          //       <146, 150, 154>
                                          //       <147, 151, 155>
  float4 dummy5;
  row_major float2x3 v;                   // covers 2 float4s with padding at end of each row (but not column)
                                          // row0: {160, 161, 162}, <163>
                                          // row1: {164, 165, 166}, <167>
  float4 dummy6;

  column_major float2x2 w;                // covers 2 float4s with padding at end of each column (but not row)
                                          // row0: {168, 172, 172}
                                          // row1: {169, 173, 173}
                                          //       <170, 174, 174>
                                          //       <171, 175, 175>
  float4 dummy7;
  row_major float2x2 x;                   // covers 2 float4s with padding at end of each row (but not column)
                                          // row1: {184, 185}, <186, 187>
                                          // row1: {188, 189}, <190, 191>
  float4 dummy8;

  row_major float2x2 y;                   // covers the same as above, but z overlaps
                                          // row0: {196, 197}, <198, 199>
                                          // row1: {200, 201}, <202, 203>
  float z;                                // overlaps after padding in final row = 202

  float4 gldummy3;                        // account for z not overlapping in GL/VK

  row_major float4x1 aa;                  // covers 4 vec4s with maximum padding
                                          // row0: {208}, <209, 210, 211>
                                          // row1: {212}, <213, 214, 215>
                                          // row2: {216}, <217, 218, 219>
                                          // row3: {220}, <221, 222, 223>

  column_major float4x1 ab;               // covers 1 vec4 (equivalent to a plain vec4)
                                          // row0: {224}
                                          // row1: {225}
                                          // row2: {226}
                                          // row3: {227}

  float4 test;                            // {228, 229, 230, 231}
};

float4 main(v2f IN) : SV_Target0
{
	return test;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;
  ID3D11BufferPtr cb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11Texture2DPtr fltTex;
  ID3D11RenderTargetViewPtr fltRT;
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

  Vec4f cbufferdata[256];

  for(int i = 0; i < 256; i++)
    cbufferdata[i] = Vec4f(float(i * 4 + 0), float(i * 4 + 1), float(i * 4 + 2), float(i * 4 + 3));

  if(MakeBuffer(eCBuffer, 0, sizeof(cbufferdata), 0, DXGI_FORMAT_UNKNOWN, cbufferdata, &cb, NULL,
                NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  MakeTexture2D(screenWidth, screenHeight, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &fltTex, NULL, NULL,
                &fltRT, NULL);

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

    ctx->PSSetConstantBuffers(0, 1, &cb.GetInterfacePtr());

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &fltRT.GetInterfacePtr(), NULL);

    ctx->Draw(3, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "CBuffer_Zoo",
              "Tests every kind of constant that can be in a cbuffer to make sure it's decoded "
              "correctly");