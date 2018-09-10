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

struct Empty_Compute_Dispatch : D3D11GraphicsTest
{
  static constexpr char *Description = "Test dispatching with one threadgroup count set to 0";

  string compute = R"EOSHADER(

RWBuffer<uint4> buffer : register(u0);

[numthreads(1,1,1)]
void main()
{
	buffer[0] = uint4(1,2,3,4);
}

)EOSHADER";

  int main(int argc, char **argv)
  {
    // initialise, create window, create device, etc
    if(!Init(argc, argv))
      return 3;

    HRESULT hr = S_OK;

    ID3DBlobPtr csblob = Compile(compute, "main", "cs_5_0");

    ID3D11ComputeShaderPtr cs;
    CHECK_HR(dev->CreateComputeShader(csblob->GetBufferPointer(), csblob->GetBufferSize(), NULL, &cs));

    uint32_t data[16] = {0};

    ID3D11BufferPtr buf;
    ID3D11UnorderedAccessViewPtr uav;
    if(MakeBuffer(eCompBuffer, 0, sizeof(data), sizeof(uint32_t) * 4, DXGI_FORMAT_R32G32B32A32_UINT,
                  data, &buf, NULL, &uav, NULL))
    {
      TEST_ERROR("Failed to create compute UAV");
      return 1;
    }

    while(Running())
    {
      float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
      ctx->ClearRenderTargetView(bbRTV, col);

      ctx->CSSetShader(cs, NULL, 0);

      UINT val = 0;
      ctx->CSSetUnorderedAccessViews(0, 1, &uav.GetInterfacePtr(), &val);

      ctx->Dispatch(1, 1, 0);

      vector<byte> contents = GetBufferData(buf, 0, 0);

      uint32_t *u32 = (uint32_t *)&contents[0];

      TEST_LOG("Data: %u %u %u %u", u32[0], u32[1], u32[2], u32[3]);

      Present();
    }

    return 0;
  }
};

REGISTER_TEST(Empty_Compute_Dispatch);