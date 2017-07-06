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
string compute = R"EOSHADER(

ByteAddressBuffer inbuf : register(t0);
RWByteAddressBuffer outbuf : register(u0);

[numthreads(1, 1, 1)]
void main()
{
	uint4 data = inbuf.Load4(5*4);
	outbuf.Store4(10*4, data);

	data.xy = inbuf.Load2(15*4);
	outbuf.Store2(0, data.xy);
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11BufferPtr buf;
  ID3D11UnorderedAccessViewPtr uav;

  ID3D11BufferPtr buf2;
  ID3D11ShaderResourceViewPtr srv;

  ID3D11ComputeShaderPtr cs;
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr csblob = Compile(compute, "main", "cs_5_0");

  CHECK_HR(dev->CreateComputeShader(csblob->GetBufferPointer(), csblob->GetBufferSize(), NULL, &cs));

  if(MakeBuffer(BufType(eCompBuffer | eRawBuffer), 0, sizeof(uint32_t) * 128, sizeof(uint32_t),
                DXGI_FORMAT_R32_TYPELESS, NULL, &buf, NULL, &uav, NULL))
  {
    TEST_ERROR("Failed to create UAV");
    return 1;
  }

  if(MakeBuffer(BufType(eCompBuffer | eRawBuffer), 0, sizeof(uint32_t) * 128, sizeof(uint32_t),
                DXGI_FORMAT_R32_TYPELESS, NULL, &buf2, &srv, NULL, NULL))
  {
    TEST_ERROR("Failed to create UAV");
    return 1;
  }

  uint32_t data[128] = {};
  for(int i = 0; i < 128; i++)
    data[i] = (uint32_t)rand();

  ctx->UpdateSubresource(buf2, 0, NULL, data, sizeof(uint32_t) * 128, sizeof(uint32_t) * 128);

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    ctx->ClearUnorderedAccessViewUint(uav, (uint32_t *)col);

    ctx->CSSetShaderResources(0, 1, &srv.GetInterfacePtr());
    ctx->CSSetUnorderedAccessViews(0, 1, &uav.GetInterfacePtr(), NULL);
    ctx->CSSetShader(cs, NULL, 0);

    ctx->Dispatch(1, 1, 1);

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11", "Byte_Address_Buffers",
              "Tests reading and writing from byte address buffers");