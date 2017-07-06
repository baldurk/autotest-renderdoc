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

float4 main(v2f IN) : SV_Target0
{
	uint2 p = uint2(IN.pos.xy) & 1;

	float4 ret = float4(0, 0, 0, 1);

	// cause quad to repeatedly diverge
  // in different ways to make sure we always have correct derivatives

	// first just a single if
	[branch]
	if(p.x == 0)
	{
		ret.x += sin(cos(pow(abs(IN.uv.y), 1.0f/3.85f)));
		ret.y += cos(sin(pow(abs(IN.uv.x), 1.0f/5.0111f)));
	}

	ret.z += 1001.0f*ddx(ret.x);
	ret.w += 1002.0f*ddx(ret.y);
	ret.z += 1003.0f*ddy(ret.x);
	ret.w += 1004.0f*ddy(ret.y);

	// next an if/else
	[branch]
	if(p.y == 0)
	{
		ret.x += sin(cos(pow(abs(IN.uv.y), 1.0f/10.15f)));
		ret.y += cos(sin(pow(abs(IN.uv.x), 1.0f/9.005f)));
	}
	else
	{
		ret.x += cos(sin(pow(abs(IN.uv.y), 1.0f/11.17f)));
		ret.y += sin(cos(pow(abs(IN.uv.x), 1.0f/8.2f)));
	}

	ret.z += 101.0f*ddx(ret.x);
	ret.w += 102.0f*ddx(ret.y);
	ret.z += 103.0f*ddy(ret.x);
	ret.w += 104.0f*ddy(ret.y);

	// now a loop with a different loop count over the quad
	[loop]
	for(uint i=0; i < (1 + 3*p.x + 5*p.y); i++)
	{
		float2 prev = ret.xy;

		ret.x = sin(prev.y);
		ret.y = cos(prev.x);
	}

	ret.z += 11.0f*ddx(ret.x);
	ret.w += 12.0f*ddx(ret.y);
	ret.z += 13.0f*ddy(ret.x);
	ret.w += 14.0f*ddy(ret.y);

	// finally a switch
	[branch]
	switch(p.x + p.y)
	{
		case 1:
		{
			float2 prev = ret.xy;
			ret.x = 2.0f*prev.y;
			ret.y = 2.0f*prev.x;
			break;
		}
		// case 0 and 2
		default:
		{
			float2 prev = ret.xy;
			ret.x = 0.7f*prev.x;
			ret.y = 0.7f*prev.y;
			break;
		}
	}

	ret.z += 1.0f*ddx(ret.x);
	ret.w += 2.0f*ddx(ret.y);
	ret.z += 3.0f*ddy(ret.x);
	ret.w += 4.0f*ddy(ret.y);

	return ret;
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11InputLayoutPtr layout;
  ID3D11BufferPtr vb;

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

    D3D11_VIEWPORT view = {0.0f, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &bbRTV.GetInterfacePtr(), NULL);

    ctx->Draw(3, 0);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int D3D11_Divergent_Shader(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}