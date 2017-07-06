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

string compute = R"EOSHADER(

RWBuffer<uint4> uav : register(u20);

[numthreads(1, 1, 1)]
void main()
{
	uav[0] = uint4(7,8,9,10);
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
  int main(int argc, char **argv);

  ID3D11UnorderedAccessViewPtr uav;

  ID3D11BufferPtr buf;
  ID3D11ComputeShaderPtr cs;
};

int impl::main(int argc, char **argv)
{
  d3d11_1 = true;

  // initialise, create window, create device, etc
  if(!Init(argc, argv))
    return 3;

  HRESULT hr = S_OK;

  ID3DBlobPtr csblob = Compile(compute, "main", "cs_5_0");

  CHECK_HR(dev->CreateComputeShader(csblob->GetBufferPointer(), csblob->GetBufferSize(), NULL, &cs));

  if(MakeBuffer(eCompBuffer, 0, sizeof(uint32_t) * 4, sizeof(uint32_t) * 4,
                DXGI_FORMAT_R32G32B32A32_UINT, NULL, &buf, NULL, &uav, NULL))
  {
    TEST_ERROR("Failed to create UAV");
    return 1;
  }

  for(int frame = 0; frame < 10 && Running(); frame++)
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(bbRTV, col);

    ctx->ClearUnorderedAccessViewUint(uav, (uint32_t *)col);

    ctx->CSSetUnorderedAccessViews(20, 1, &uav.GetInterfacePtr(), NULL);
    ctx->CSSetShader(cs, NULL, 0);

    ctx->Dispatch(1, 1, 1);

    vector<byte> contents = GetBufferData(buf);

    uint32_t *u32 = (uint32_t *)&contents[0];

    TEST_LOG("Data: %u %u %u %u", u32[0], u32[1], u32[2], u32[3]);

    Present();
  }

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("D3D11_1", "Many_UAVs",
              "Test using more than 8 compute shader UAVs (D3D11.1 feature)");