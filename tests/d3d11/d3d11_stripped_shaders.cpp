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

struct Stripped_Shaders : D3D11GraphicsTest
{
  static constexpr char *Description =
      "Tests shaders with their debug/reflection info stripped out and stored in separate blobs";

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
	return IN.col;
}

)EOSHADER";

  int main(int argc, char **argv)
  {
    // initialise, create window, create device, etc
    if(!Init(argc, argv))
      return 3;

    HRESULT hr = S_OK;

    ID3DBlobPtr vsblobUnstripped = NULL;
    ID3DBlobPtr psblobUnstripped = NULL;

    ID3DBlobPtr vsblob = Compile(common + vertex, "main", "vs_5_0", &vsblobUnstripped);
    ID3DBlobPtr psblob = Compile(common + pixel, "main", "ps_5_0", &psblobUnstripped);

    WriteBlob(GetCWD() + "/shader_debug.vs", vsblobUnstripped, false);
    WriteBlob(GetCWD() + "/shader_debug.ps", psblobUnstripped, true);

    vsblob = SetBlobPath(GetCWD() + "/shader_debug.vs", vsblob);

    CreateDefaultInputLayout(vsblob);

    ID3D11VertexShaderPtr vs;
    CHECK_HR(dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
    ID3D11PixelShaderPtr ps;
    CHECK_HR(dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

    SetBlobPath("lz4#shader_debug.ps", ps);

    ID3D11BufferPtr vb;
    if(MakeBuffer(eVBuffer, 0, sizeof(DefaultTri), 0, DXGI_FORMAT_UNKNOWN, DefaultTri, &vb, NULL,
                  NULL, NULL))
    {
      TEST_ERROR("Failed to create triangle VB");
      return 1;
    }

    while(Running())
    {
      float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
      ctx->ClearRenderTargetView(bbRTV, col);

      UINT stride = sizeof(DefaultA2V);
      UINT offset = 0;
      ctx->IASetVertexBuffers(0, 1, &vb.GetInterfacePtr(), &stride, &offset);
      ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      ctx->IASetInputLayout(defaultLayout);

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
};

REGISTER_TEST(Stripped_Shaders);