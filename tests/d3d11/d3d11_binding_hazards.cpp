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

string compute = R"EOSHADER(

Texture2D<uint> texin : register(t0);
Buffer<uint> bufin : register(t1);
RWTexture2D<uint> texout1 : register(u0);
RWBuffer<uint> bufout1 : register(u1);
RWTexture2D<uint> texout2 : register(u2);
RWBuffer<uint> bufout2 : register(u3);

[numthreads(1,1,1)]
void main()
{
	texout1[uint2(3,4)] = bufin[3];
	texout2[uint2(4,4)] = texin[uint2(3,3)];
	bufout1[4] = bufin[4];
	bufout2[3] = texin[uint2(4,4)];
}

)EOSHADER";

struct impl : D3D11GraphicsTest
{
	int main(int argc, char **argv);

	ID3D11BufferPtr buf;
	ID3D11BufferPtr buf2;
	ID3D11Texture2DPtr tex[2];
	ID3D11ShaderResourceViewPtr srv[3];
	ID3D11UnorderedAccessViewPtr uav[3];
	ID3D11RenderTargetViewPtr rtv[3];

	ID3D11ComputeShaderPtr cs;
};

int impl::main(int argc, char **argv)
{
  // so that running individually we get errors
  debugDevice = true;
  LoadLibraryA("C:/projects/renderdoc/x64/development/renderdoc.dll");

	// initialise, create window, create device, etc
	if(!Init(argc, argv))
		return 3;

	HRESULT hr = S_OK;

	ID3DBlobPtr csblob = Compile(compute, "main", "cs_5_0");
	
	CHECK_HR(dev->CreateComputeShader(csblob->GetBufferPointer(), csblob->GetBufferSize(), NULL, &cs));

	for(int i=0; i < 2; i++)
	{
		if(MakeTexture2D(8, 8, 1, DXGI_FORMAT_R32_UINT, &tex[i], &srv[i], &uav[i], &rtv[i], NULL))
		{
			TEST_ERROR("Failed to create compute tex");
			return 1;
		}
	}

  MakeBuffer(eCompBuffer, 0, 65536, 4, DXGI_FORMAT_R32_UINT, NULL, &buf, &srv[2], &uav[2], NULL);
  MakeBuffer(eCompBuffer, 0, 65536, 4, DXGI_FORMAT_R32_UINT, NULL, &buf2, (ID3D11ShaderResourceView **)0x1, NULL, NULL);
  
	while(Running())
  {
    ctx->ClearState();

    ctx->CSSetShader(cs, NULL, 0);
    
		CD3D11_SHADER_RESOURCE_VIEW_DESC sdesc(D3D11_SRV_DIMENSION_BUFFER, DXGI_FORMAT_R32_UINT, 0, 128);

    ID3D11ShaderResourceView *tempSRV = NULL;

    dev->CreateShaderResourceView(buf2, &sdesc, &tempSRV);

    ctx->CSSetShaderResources(1, 1, &tempSRV);

    ULONG refcount = tempSRV->Release();

    ID3D11ShaderResourceView *srvs[2] = {
       NULL,
       tempSRV
    };

    ctx->CSSetShaderResources(1, 2, srvs);

    refcount = tempSRV->AddRef();
    refcount = tempSRV->Release();
    
    ctx->CSSetShaderResources(3, 2, srvs);

    refcount = tempSRV->AddRef();
    refcount = tempSRV->Release();

    ctx->CSSetUnorderedAccessViews(0, 1, &uav[0].GetInterfacePtr(), NULL);
    ctx->CSSetUnorderedAccessViews(2, 1, &uav[1].GetInterfacePtr(), NULL);

    // try to bind the buffer to two slots, find it gets unbound
    ctx->CSSetUnorderedAccessViews(1, 1, &uav[2].GetInterfacePtr(), NULL);
    ctx->CSSetUnorderedAccessViews(3, 1, &uav[2].GetInterfacePtr(), NULL);

    ID3D11UnorderedAccessView *getCSUAVs[4] = {};
    ID3D11RenderTargetView *getOMRTV = NULL;
    ID3D11RenderTargetView *getOMRTVs[2] = {};
    ID3D11UnorderedAccessView *getOMUAV = NULL;

    // Dispatch each time so we can also check state in UI
    ctx->Dispatch(1, 1, 1);
    ctx->CSGetUnorderedAccessViews(0, 4, getCSUAVs);

    TEST_ASSERT(getCSUAVs[0] == uav[0], "Unexpected binding");
    TEST_ASSERT(getCSUAVs[1] == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[2] == uav[1], "Unexpected binding");
    TEST_ASSERT(getCSUAVs[3] == uav[2], "Unexpected binding");

    // this should unbind uav[0] because it's re-bound as rtv[0], then unbind uav[1] because it's rebound on another UAV slot
    ctx->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtv[0].GetInterfacePtr(), NULL, 1, 1, &uav[1].GetInterfacePtr(), NULL);

    ctx->Dispatch(1, 1, 1);
    ctx->OMGetRenderTargetsAndUnorderedAccessViews(1, &getOMRTV, NULL, 1, 1, &getOMUAV);
    ctx->CSGetUnorderedAccessViews(0, 4, getCSUAVs);
    
    TEST_ASSERT(getOMRTV == rtv[0], "Unexpected binding");
    TEST_ASSERT(getOMUAV == uav[1], "Unexpected binding");
    TEST_ASSERT(getCSUAVs[0] == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[1] == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[2] == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[3] == uav[2], "Unexpected binding");

    // finally this should unbind both OM views, and rebind back on the CS
    ctx->CSSetUnorderedAccessViews(0, 1, &uav[0].GetInterfacePtr(), NULL);
    ctx->CSSetUnorderedAccessViews(2, 1, &uav[1].GetInterfacePtr(), NULL);

    ctx->Dispatch(1, 1, 1);
    ctx->OMGetRenderTargetsAndUnorderedAccessViews(1, &getOMRTV, NULL, 1, 1, &getOMUAV);
    ctx->CSGetUnorderedAccessViews(0, 4, getCSUAVs);

    TEST_ASSERT(getOMRTV == NULL, "Unexpected binding");
    TEST_ASSERT(getOMUAV == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[0] == uav[0], "Unexpected binding");
    TEST_ASSERT(getCSUAVs[1] == NULL, "Unexpected binding");
    TEST_ASSERT(getCSUAVs[2] == uav[1], "Unexpected binding");
    TEST_ASSERT(getCSUAVs[3] == uav[2], "Unexpected binding");

    ctx->ClearState();

    ctx->CSSetShader(cs, NULL, 0);
    
    ID3D11RenderTargetView *RTVs[] = { 
      rtv[0],
      rtv[0],
    };

    // can't bind the same RTV to two slots
    ctx->OMSetRenderTargets(2, RTVs, NULL);
    
    ctx->Dispatch(1, 1, 1);
    ctx->OMGetRenderTargetsAndUnorderedAccessViews(2, getOMRTVs, NULL, 2, 1, &getOMUAV);
    TEST_ASSERT(getOMRTVs[0] == NULL, "Unexpected binding");
    TEST_ASSERT(getOMRTVs[1] == NULL, "Unexpected binding");
    TEST_ASSERT(getOMUAV == NULL, "Unexpected binding");

    RTVs[0] = rtv[1];
    RTVs[1] = rtv[0];
    
    // this bind is fine, no overlapping state
    ctx->OMSetRenderTargetsAndUnorderedAccessViews(2, RTVs, NULL, 2, 1, &uav[2].GetInterfacePtr(), NULL);
    
    ctx->Dispatch(1, 1, 1);
    ctx->OMGetRenderTargetsAndUnorderedAccessViews(2, getOMRTVs, NULL, 2, 1, &getOMUAV);
    TEST_ASSERT(getOMRTVs[0] == rtv[1], "Unexpected binding");
    TEST_ASSERT(getOMRTVs[1] == rtv[0], "Unexpected binding");
    TEST_ASSERT(getOMUAV == uav[2], "Unexpected binding");

    RTVs[0] = rtv[0];
    RTVs[1] = rtv[1];
    
    // this bind is discarded, because RTV[0] overlaps UAV[0].
    ctx->OMSetRenderTargetsAndUnorderedAccessViews(2, RTVs, NULL, 2, 1, &uav[0].GetInterfacePtr(), NULL);
    
    ctx->Dispatch(1, 1, 1);
    ctx->OMGetRenderTargetsAndUnorderedAccessViews(2, getOMRTVs, NULL, 2, 1, &getOMUAV);
    TEST_ASSERT(getOMRTVs[0] == rtv[1], "Unexpected binding");
    TEST_ASSERT(getOMRTVs[1] == rtv[0], "Unexpected binding");
    TEST_ASSERT(getOMUAV == uav[2], "Unexpected binding");

    Present();
  }

	return 0;
}

}; // anonymous namespace

int D3D11_Binding_Hazards(int argc, char **argv) { impl i; return i.main(argc, argv); }