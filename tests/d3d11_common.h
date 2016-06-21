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

#pragma once

#include "test_common.h"

#include <windows.h>
#include <comdef.h>

#include <dxgi.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

#include <d3dcompiler.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#define COM_SMARTPTR(classname) _COM_SMARTPTR_TYPEDEF(classname, __uuidof(classname))

COM_SMARTPTR(ID3DBlob);
COM_SMARTPTR(IDXGISwapChain);

COM_SMARTPTR(ID3D11Device);
COM_SMARTPTR(ID3D11Device1);
COM_SMARTPTR(ID3D11Device2);

COM_SMARTPTR(ID3D11DeviceContext);
COM_SMARTPTR(ID3D11DeviceContext1);
COM_SMARTPTR(ID3D11DeviceContext2);

COM_SMARTPTR(ID3D11CommandList);

COM_SMARTPTR(ID3D11InputLayout);

COM_SMARTPTR(ID3D11Buffer);

COM_SMARTPTR(ID3D11VertexShader);
COM_SMARTPTR(ID3D11PixelShader);
COM_SMARTPTR(ID3D11HullShader);
COM_SMARTPTR(ID3D11DomainShader);
COM_SMARTPTR(ID3D11GeometryShader);
COM_SMARTPTR(ID3D11ComputeShader);

COM_SMARTPTR(ID3D11RasterizerState);
COM_SMARTPTR(ID3D11BlendState);
COM_SMARTPTR(ID3D11DepthStencilState);
COM_SMARTPTR(ID3D11SamplerState);

COM_SMARTPTR(ID3D11Texture1D);
COM_SMARTPTR(ID3D11Texture2D);
COM_SMARTPTR(ID3D11Texture3D);
COM_SMARTPTR(ID3D11RenderTargetView);
COM_SMARTPTR(ID3D11ShaderResourceView);
COM_SMARTPTR(ID3D11UnorderedAccessView);
COM_SMARTPTR(ID3D11DepthStencilView);

COM_SMARTPTR(ID3DUserDefinedAnnotation);

#define CHECK_HR(expr)       { hr = (expr); if( FAILED(hr) ) { TEST_ERROR( "Failed HRESULT at %s:%d (%x): %s", __FILE__, (int)__LINE__, hr, #expr ); DEBUG_BREAK(); exit(1); } }

struct D3D11GraphicsTest : public GraphicsTest
{
	D3D11GraphicsTest()
		: backbufferFmt(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		, backbufferCount(2)
		, backbufferMSAA(1)
		, d3d11_1(false)
		, d3d11_2(false)
		, wnd(NULL)
	{
	}

	~D3D11GraphicsTest();

	bool Init(int argc, char **argv);
	
	enum BufType
	{
		eCBuffer = 0x0,
		eStageBuffer = 0x1,
		eVBuffer = 0x2,
		eIBuffer = 0x4,
		eBuffer = 0x8,
		eCompBuffer = 0x10,
		eSOBuffer = 0x20,
		BufMajorType = 0xff,

		eAppend = 0x100,
		BufUAVType = 0xf00,
	};

	ID3DBlobPtr Compile(string src, string entry, string profile, ID3DBlob **unstripped = NULL);
	void WriteBlob(string name, ID3DBlob *blob, bool compress);

	ID3DBlobPtr SetBlobPath(string name, ID3DBlob *blob);
	void SetBlobPath(string name, ID3D11DeviceChild *shader);
	
	int MakeBuffer(BufType type, UINT flags, UINT byteSize, UINT structSize, DXGI_FORMAT fmt, void *data, ID3D11Buffer **buf,
					ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav, ID3D11RenderTargetView **rtv);
	
	int MakeTexture2D(UINT w, UINT h, UINT mips, DXGI_FORMAT fmt, ID3D11Texture2D **tex,
						ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav,
						ID3D11RenderTargetView **rtv, ID3D11DepthStencilView **dsv);
	int MakeTexture2DMS(UINT w, UINT h, UINT mips, UINT sampleCount, DXGI_FORMAT fmt, ID3D11Texture2D **tex,
						ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav,
						ID3D11RenderTargetView **rtv, ID3D11DepthStencilView **dsv);
	
	vector<byte> GetBufferData(ID3D11Buffer *buf, uint32_t offset = 0, uint32_t len = 0);

	D3D11_MAPPED_SUBRESOURCE Map(ID3D11Resource *res, UINT sub, D3D11_MAP type)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		ctx->Map(res, sub, type, 0, &mapped);
		return mapped;
	}

	bool Running();
	void Present();

	DXGI_FORMAT backbufferFmt;
	int backbufferCount;
	int backbufferMSAA;
	bool d3d11_1;
	bool d3d11_2;

	HWND wnd;

	IDXGISwapChainPtr swap;

	ID3D11Texture2DPtr bbTex;
	ID3D11RenderTargetViewPtr bbRTV;

	ID3D11DevicePtr dev;
	ID3D11Device1Ptr dev1;
	ID3D11Device2Ptr dev2;

	ID3D11DeviceContextPtr ctx;
	ID3D11DeviceContext1Ptr ctx1;
	ID3D11DeviceContext2Ptr ctx2;
	ID3DUserDefinedAnnotationPtr annot;
};