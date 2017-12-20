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
  float zero;
  float one;
  float negone;
};

string common = R"EOSHADER(

struct a2v
{
	float3 pos : POSITION;
	float zeroVal : ZERO;
	float oneVal : ONE;
	float negoneVal : NEGONE;
};

struct v2f
{
	float4 pos : SV_POSITION;
	float2 zeroVal : ZERO;
	float oneVal : ONE;
	float negoneVal : NEGONE;
	uint tri : TRIANGLE;
};

)EOSHADER";

string vertex = R"EOSHADER(

v2f main(a2v IN, uint tri : SV_InstanceID)
{
	v2f OUT = (v2f)0;

	OUT.pos = float4(IN.pos.xyz * 0.1f, 1);

	uint row = tri / 10;
	uint col = tri % 10;

	OUT.pos.x += -0.9f + col*0.2f;
	OUT.pos.y +=  0.85f - row*0.3f;

	OUT.zeroVal = IN.zeroVal.xx;
	OUT.oneVal = IN.oneVal;
	OUT.negoneVal = IN.negoneVal;
	OUT.tri = tri;

	return OUT;
}

)EOSHADER";

string pixel = R"EOSHADER(

Buffer<float> test : register(t0);

float4 main(v2f IN) : SV_Target0
{
  float	posinf = IN.oneVal/IN.zeroVal.x;
  float	neginf = IN.negoneVal/IN.zeroVal.x;
  float	nan = IN.zeroVal.x/IN.zeroVal.y;

	float negone = IN.negoneVal;
	float posone = IN.oneVal;
	float zero = IN.zeroVal.x;

	if(IN.tri == 0)
		return float4(log(negone), log(zero), log(posone), 1.0f);
	if(IN.tri == 1)
		return float4(log(posinf), log(neginf), log(nan), 1.0f);
	if(IN.tri == 2)
		return float4(exp(negone), exp(zero), exp(posone), 1.0f);
	if(IN.tri == 3)
		return float4(exp(posinf), exp(neginf), exp(nan), 1.0f);
	if(IN.tri == 4)
		return float4(sqrt(negone), sqrt(zero), sqrt(posone), 1.0f);
	if(IN.tri == 5)
		return float4(sqrt(posinf), sqrt(neginf), sqrt(nan), 1.0f);
	if(IN.tri == 6)
		return float4(rsqrt(negone), rsqrt(zero), rsqrt(posone), 1.0f);
	if(IN.tri == 7)
		return float4(saturate(posinf), saturate(neginf), saturate(nan), 1.0f);
	if(IN.tri == 8)
		return float4(min(posinf, nan), min(neginf, nan), min(nan, nan), 1.0f);
	if(IN.tri == 9)
		return float4(min(posinf, posinf), min(neginf, posinf), min(nan, posinf), 1.0f);
	if(IN.tri == 10)
		return float4(min(posinf, neginf), min(neginf, neginf), min(nan, neginf), 1.0f);
	if(IN.tri == 11)
		return float4(max(posinf, nan), max(neginf, nan), max(nan, nan), 1.0f);
	if(IN.tri == 12)
		return float4(max(posinf, posinf), max(neginf, posinf), max(nan, posinf), 1.0f);
	if(IN.tri == 13)
		return float4(max(posinf, neginf), max(neginf, neginf), max(nan, neginf), 1.0f);

  // rounding tests
  float round_a = 1.7f*posone;
  float round_b = 2.1f*posone;
  float round_c = 1.5f*posone;
  float round_d = 2.5f*posone;
  float round_e = zero;
  float round_f = -1.7f*posone;
  float round_g = -2.1f*posone;
  float round_h = -1.5f*posone;
  float round_i = -2.5f*posone;

  if(IN.tri == 14)
    return float4(round(round_a), floor(round_a), ceil(round_a), trunc(round_a));
  if(IN.tri == 15)
    return float4(round(round_b), floor(round_b), ceil(round_b), trunc(round_b));
  if(IN.tri == 16)
    return float4(round(round_c), floor(round_c), ceil(round_c), trunc(round_c));
  if(IN.tri == 17)
    return float4(round(round_d), floor(round_d), ceil(round_d), trunc(round_d));
  if(IN.tri == 18)
    return float4(round(round_e), floor(round_e), ceil(round_e), trunc(round_e));
  if(IN.tri == 19)
    return float4(round(round_f), floor(round_f), ceil(round_f), trunc(round_f));
  if(IN.tri == 20)
    return float4(round(round_g), floor(round_g), ceil(round_g), trunc(round_g));
  if(IN.tri == 21)
    return float4(round(round_h), floor(round_h), ceil(round_h), trunc(round_h));
  if(IN.tri == 22)
    return float4(round(round_i), floor(round_i), ceil(round_i), trunc(round_i));

  if(IN.tri == 23)
    return float4(round(neginf), floor(neginf), ceil(neginf), trunc(neginf));
  if(IN.tri == 24)
    return float4(round(posinf), floor(posinf), ceil(posinf), trunc(posinf));
  if(IN.tri == 25)
    return float4(round(nan), floor(nan), ceil(nan), trunc(nan));

  if(IN.tri == 26)
    return test[5].xxxx;

	return float4(0.4f, 0.4f, 0.4f, 0.4f);
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11BufferPtr srvBuf;
  ID3D11ShaderResourceViewPtr srv;

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
          "ZERO", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
          D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
      {
          "ONE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
          D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
      {
          "NEGONE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
          D3D11_INPUT_PER_VERTEX_DATA, 0,
      },
  };

  CHECK_HR(dev->CreateInputLayout(layoutdesc, ARRAY_COUNT(layoutdesc), vsblob->GetBufferPointer(),
                                  vsblob->GetBufferSize(), &layout));

  CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  MakeTexture2D(screenWidth, screenHeight, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &fltTex, NULL, NULL,
                &fltRT, NULL);

  a2v triangle[] = {
      {
          Vec3f(-0.5f, 0.0f, 0.0f), 0.0f, 1.0f, -1.0f,
      },
      {
          Vec3f(0.0f, 1.0f, 0.0f), 0.0f, 1.0f, -1.0f,
      },
      {
          Vec3f(0.5f, 0.0f, 0.0f), 0.0f, 1.0f, -1.0f,
      },
  };

  if(MakeBuffer(eVBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle, &vb, NULL, NULL,
                NULL))
  {
    TEST_ERROR("Failed to create triangle VB");
    return 1;
  }

  float testdata[] = {
      1.0f,  2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,  9.0f,  10.0f,
      11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 120.0f,
  };

  MakeBuffer(eCompBuffer, 0, sizeof(testdata), 10, DXGI_FORMAT_UNKNOWN, testdata, &srvBuf, &srv,
             NULL, NULL);

  ctx->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(fltRT, col);
    ctx->ClearRenderTargetView(bbRTV, col);

    UINT stride = sizeof(a2v);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(layout);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &fltRT.GetInterfacePtr(), NULL);

    ctx->DrawInstanced(3, 70, 0, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Debug_Shader",
              "Tests simple shader debugging identities by rendering many small triangles and "
              "performing one calculation to each to an F32 target");