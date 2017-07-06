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

namespace {

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

Texture3D<float> tex : register(t0);
SamplerState samp : register(s0);

float4 main(v2f IN) : SV_Target0
{
  float4 ret = 0.0f;
  float mul = 0.5f;
  const float step = 0.5f;

  ret += tex.SampleLevel(samp, IN.uv.yxx, 7.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.xyx, 6.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.xxy, 5.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.yxy, 4.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.yyx, 3.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.xyy, 2.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.yyy, 1.0f) * mul; mul *= step;
  ret += tex.SampleLevel(samp, IN.uv.xxx, 0.0f) * mul;

  return (ret + 0.5f) * IN.col;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11SamplerStatePtr samp;
  ID3D11Texture3DPtr tex;
  ID3D11ShaderResourceViewPtr srv;
};

int impl::main(int argc, char **argv)
{
  debugDevice = true;

  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = Compile(common + vertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = Compile(common + pixel, "main", "ps_5_0");

  D3D11_INPUT_ELEMENT_DESC layoutdesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0, },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0, },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0, },
  };

  CHECK_HR(dev->CreateInputLayout(layoutdesc, ARRAY_COUNT(layoutdesc), vsblob->GetBufferPointer(), vsblob->GetBufferSize(), &layout));

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  CD3D11_SAMPLER_DESC sampdesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
  CHECK_HR(dev->CreateSamplerState(&sampdesc, &samp));

  CD3D11_TEXTURE3D_DESC texdesc = CD3D11_TEXTURE3D_DESC(DXGI_FORMAT_R8_UNORM, 128, 128, 1024, 8);
  CHECK_HR(dev->CreateTexture3D(&texdesc, NULL, &tex));

  uint8_t *data = new uint8_t[128 * 128 * 1024 * sizeof(uint8_t)];

  char *digits[10] = {

    "..####.."
    ".#....#."
    "#......#"
    "#......#"       // 0
    "#......#"
    "#......#"
    ".#....#."
    "..####..",

    "....#..."
    "...##..."
    "..#.#..."
    "....#..."       // 1
    "....#..."
    "....#..."
    "....#..."
    "..####..",

    "..###..."
    ".#...#.."
    ".....#.."
    "....#..."       // 2
    "....#..."
    "...#...."
    "...#...."
    "..####..",

    "..###..."
    ".#...#.."
    ".....#.."
    ".....#.."       // 3
    "..###..."
    ".....#.."
    ".#...#.."
    "..###...",

    "........"
    "....#..."
    "...#...."
    "..#....."
    ".#..#..."       // 4
    ".#####.."
    "....#..."
    "....#...",

    ".#####.."
    ".#......"
    ".#......"
    ".####..."       // 5
    ".....#.."
    ".....#.."
    ".#...#.."
    "..###...",

    "........"
    ".....#.."
    "....#..."
    "...#...."
    "..####.."       // 6
    ".#....#."
    ".#....#."
    "..####..",

    "........"
    "........"
    ".######."
    ".....#.."
    "....#..."       // 7
    "...#...."
    "..#....."
    ".#......",

    "..####.."
    ".#....#."
    ".#....#."
    "..####.."       // 8
    ".#....#."
    ".#....#."
    ".#....#."
    "..####..",

    "..####.."
    ".#....#."
    ".#....#."
    "..#####."       // 9
    "......#."
    ".....#.."
    "....#..."
    "...#....",
  };

  for(uint32_t mip = 0; mip < 8; mip++)
  {
    uint32_t d = 128 >> mip;

    if(mip > 0)
    {
      for(uint32_t i = 0; i < d*d*(1024>>mip); i++)
        data[i] = (rand() % 0x7fff) << 1;
    }
    else
    {
      for(uint32_t slice = 0; slice < 1024; slice++)
      {
        uint8_t *base = data + d*d*sizeof(uint8_t)*slice;

        int str[4] = {0, 0, 0, 0};

        uint32_t digitCalc = slice;

        str[0] += digitCalc/1000;

        digitCalc %= 1000;
        str[1] += digitCalc/100;

        digitCalc %= 100;
        str[2] += digitCalc/10;

        digitCalc %= 10;
        str[3] += digitCalc;

        base += 32;
        base += 32*d*sizeof(uint8_t);

        // first digit
        for(int row=0; row < 8; row++)
          memcpy(base + row*d*sizeof(uint8_t), digits[ str[0] ] + row*8, 8);

        base += 16;

        // second digit
        for(int row=0; row < 8; row++)
          memcpy(base + row*d*sizeof(uint8_t), digits[ str[1] ] + row*8, 8);

        base += 16;

        // third digit
        for(int row=0; row < 8; row++)
          memcpy(base + row*d*sizeof(uint8_t), digits[ str[2] ] + row*8, 8);

        base += 16;

        // fourth digit
        for(int row=0; row < 8; row++)
          memcpy(base + row*d*sizeof(uint8_t), digits[ str[3] ] + row*8, 8);
      }
    }

    ctx->UpdateSubresource(tex, mip, NULL, data, d*sizeof(uint8_t), d*d*sizeof(uint8_t));
  }

  CD3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(tex, DXGI_FORMAT_R8_UNORM);
  CHECK_HR(dev->CreateShaderResourceView(tex, &srvdesc, &srv));

  delete[] data;

  a2v triangle[] = {
    { Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f), },
    { Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f), },
    { Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f), },
  };

  if(MakeBuffer(eVBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle, &vb, NULL, NULL, NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  while(Running())
  {
    float col[] = { 0.4f, 0.5f, 0.6f, 1.0f };
    ctx->ClearRenderTargetView(bbRTV, col);

    UINT stride = sizeof(a2v);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(layout);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    ctx->PSSetSamplers(0, 1, &samp.GetInterfacePtr());
    ctx->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());

    D3D11_VIEWPORT view = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f };
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);

    ctx->Draw(3, 0);

    Present();
  }

  return 0;
}

}; // anonymous namespace

int D3D11_Texture_3D(int argc, char **argv) { impl i; return i.main(argc, argv); }