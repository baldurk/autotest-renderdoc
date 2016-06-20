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

#include "d3d11_common.h"

#include "lz4.h"

#include <stdio.h>

#pragma comment(lib, "d3dcompiler.lib")

#define RENDERDOC_ShaderDebugMagicValue_struct                                \
  {                                                                           \
    0xeab25520, 0x6670, 0x4865, 0x84, 0x29, 0x6c, 0x8, 0x51, 0x54, 0x00, 0xff \
  }

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_CLOSE)   { DestroyWindow(hwnd); return 0; }
	if(msg == WM_DESTROY) { PostQuitMessage(0);  return 0; }
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3D11GraphicsTest::Init(int argc, char **argv)
{
	// parse parameters here to override parameters
	GraphicsTest::Init(argc, argv);

	// D3D11 specific can go here
	// ...

	HRESULT hr = S_OK;
	
	string classname = "renderdoc_d3d11_test";
	static int idx = 0;idx++;
	classname += '0' + idx;

	WNDCLASSEXA wc;
	wc.cbSize        = sizeof(WNDCLASSEXA);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = classname.c_str();
	wc.hIconSm       = NULL;

	if(!RegisterClassEx(&wc))
	{
		return false;
	}

	RECT rect = { 0, 0, screenWidth, screenHeight };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

	wnd = CreateWindowExA(WS_EX_CLIENTEDGE, classname.c_str(), "RenderDoc test program", WS_OVERLAPPEDWINDOW,
	                          CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, NULL, NULL);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	
	swapDesc.BufferCount = backbufferCount;
	swapDesc.BufferDesc.Format = backbufferFmt;
	swapDesc.BufferDesc.Width = screenWidth;
	swapDesc.BufferDesc.Height = screenHeight;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.SampleDesc.Count = backbufferMSAA;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.OutputWindow = wnd;
	swapDesc.Windowed = fullscreen ? FALSE : TRUE;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;

	D3D_FEATURE_LEVEL features[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_DRIVER_TYPE driver = D3D_DRIVER_TYPE_HARDWARE;

	if(d3d11_1)
	{
		features[0] = D3D_FEATURE_LEVEL_11_1;
		driver = D3D_DRIVER_TYPE_REFERENCE;
	}
	
	hr = D3D11CreateDeviceAndSwapChain(NULL, driver, NULL, debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0, features, 1, D3D11_SDK_VERSION,
										&swapDesc, &swap, &dev, NULL, &ctx);

	if(FAILED(hr))
	{
		TEST_ERROR("D3D11CreateDeviceAndSwapChain failed: %x", hr);
		return false;
	}
	
	hr = swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&bbTex);
	
	if(FAILED(hr))
	{
		TEST_ERROR("swap->GetBuffer failed: %x", hr);
		dev = NULL;
		ctx = NULL;
		swap = NULL;
		return false;
	}
	
	if(d3d11_1)
	{
		dev->QueryInterface(__uuidof(ID3D11Device1), (void **)&dev1);
		ctx->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&ctx1);
	}
	
	if(d3d11_2)
	{
		dev->QueryInterface(__uuidof(ID3D11Device2), (void **)&dev2);
		ctx->QueryInterface(__uuidof(ID3D11DeviceContext2), (void **)&ctx2);
	}

	ctx->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&annot);
	
	hr = dev->CreateRenderTargetView(bbTex, NULL, &bbRTV);

	if(FAILED(hr))
	{
		TEST_ERROR("CreateRenderTargetView failed: %x", hr);
		return false;
	}
	
	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);

	return true;
}

D3D11GraphicsTest::~D3D11GraphicsTest()
{
	if (wnd) DestroyWindow(wnd);
}

ID3DBlobPtr D3D11GraphicsTest::Compile(string src, string entry, string profile, ID3DBlob **unstripped)
{
	ID3DBlobPtr blob = NULL;
	ID3DBlobPtr error = NULL;

	HRESULT hr = D3DCompile(src.c_str(), src.length(),
		"", NULL, NULL,
		entry.c_str(), profile.c_str(),
		D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG, 0, &blob, &error);

	if(FAILED(hr))
	{
		TEST_ERROR("Failed to compile shader, error %x / %s", hr, error ? (char *)error->GetBufferPointer() : "Unknown");

		blob = NULL;
		error = NULL;
		return NULL;
	}

	if(unstripped)
	{
		blob.AddRef();
		*unstripped = blob.GetInterfacePtr();
		
		ID3DBlobPtr stripped = NULL;

		D3DStripShader(blob->GetBufferPointer(), blob->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO, &stripped);

		blob = NULL;

		return stripped;
	}

	return blob;
}

void D3D11GraphicsTest::WriteBlob(string name, ID3DBlob *blob, bool compress)
{
	FILE *f = NULL;
	fopen_s(&f, name.c_str(), "wb");

	if(f == NULL)
	{
		TEST_ERROR("Can't open blob file to write %s", name.c_str());
		return;
	}

	if(compress)
	{
		int uncompSize = (int)blob->GetBufferSize();
		char *compBuf = new char[uncompSize];

		int compressedSize = LZ4_compress_default((const char *)blob->GetBufferPointer(), compBuf, uncompSize, uncompSize);

		fwrite(compBuf, 1, compressedSize, f);

		delete[] compBuf;
	}
	else
	{
		fwrite(blob->GetBufferPointer(), 1, blob->GetBufferSize(), f);
	}

	fclose(f);
}

ID3DBlobPtr D3D11GraphicsTest::SetBlobPath(string name, ID3DBlob *blob)
{
	ID3DBlobPtr newBlob = NULL;
	
	const GUID RENDERDOC_ShaderDebugMagicValue = RENDERDOC_ShaderDebugMagicValue_struct;

	string pathData;
	for(size_t i=0; i < sizeof(RENDERDOC_ShaderDebugMagicValue); i++)
		pathData.push_back(' ');

	pathData += name;

	memcpy(&pathData[0], &RENDERDOC_ShaderDebugMagicValue, sizeof(RENDERDOC_ShaderDebugMagicValue));

	D3DSetBlobPart(blob->GetBufferPointer(), blob->GetBufferSize(), D3D_BLOB_PRIVATE_DATA, 0, pathData.c_str(), pathData.size()+1, &newBlob);

	return newBlob;
}

void D3D11GraphicsTest::SetBlobPath(string name, ID3D11DeviceChild *shader)
{
	const GUID RENDERDOC_ShaderDebugMagicValue = RENDERDOC_ShaderDebugMagicValue_struct;

	shader->SetPrivateData(RENDERDOC_ShaderDebugMagicValue, (UINT)name.size()+1, name.c_str());
}

vector<byte> D3D11GraphicsTest::GetBufferData(ID3D11Buffer *buffer, uint32_t offset, uint32_t len)
{
	D3D11_MAPPED_SUBRESOURCE mapped;

	HRESULT hr = S_OK;

	TEST_ASSERT(buffer, "buffer is NULL");

	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	if(len == 0)
		len = desc.ByteWidth-offset;

	if(len > 0 && offset+len > desc.ByteWidth)
	{
		TEST_WARN("Attempting to read off the end of the array. Will be clamped");
		len = std::min(len, desc.ByteWidth-offset);
	}

	ID3D11BufferPtr stage;

	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	CHECK_HR(dev->CreateBuffer(&desc, NULL, &stage));

	vector<byte> ret;
	ret.resize(len);

	if(len > 0)
	{
		ctx->CopyResource(stage, buffer);

		CHECK_HR(ctx->Map(stage, 0, D3D11_MAP_READ, 0, &mapped))

		memcpy(&ret[0], mapped.pData, len);

		ctx->Unmap(stage, 0);
	}

	return ret;
}

int D3D11GraphicsTest::MakeBuffer(BufType type, UINT flags, UINT byteSize, UINT structSize, DXGI_FORMAT fmt, void *data, ID3D11Buffer **buf,
								ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav, ID3D11RenderTargetView **rtv)
{
	if((type & BufMajorType) == eCBuffer)
	{
		byteSize = (byteSize + 15) & ~0xf;
	}

	D3D11_BUFFER_DESC bufDesc;
	bufDesc.ByteWidth = byteSize;
	bufDesc.MiscFlags = flags;
	bufDesc.StructureByteStride = structSize;

	if(structSize > 0 && fmt == DXGI_FORMAT_UNKNOWN)
		bufDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	else
		bufDesc.StructureByteStride = 0;

	if(structSize > 0 && (byteSize % structSize) != 0)
		TEST_FATAL("Invalid structure size - not divisor of byte size");

	switch(type & BufMajorType)
	{
		case eCBuffer:
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.Usage = D3D11_USAGE_DEFAULT;
			break;
		case eStageBuffer:
			bufDesc.BindFlags = 0;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
			bufDesc.Usage = D3D11_USAGE_STAGING;
			break;
		case eVBuffer:
			bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case eIBuffer:
			bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case eBuffer:
			bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case eSOBuffer:
			bufDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.Usage = D3D11_USAGE_DEFAULT;
			break;
		case eCompBuffer:
			bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.Usage = D3D11_USAGE_DEFAULT;
			break;
	}

	if(srv)
		bufDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if(uav)
		bufDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	if(rtv)
		bufDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	D3D11_SUBRESOURCE_DATA initdata;
	initdata.pSysMem = data;
	initdata.SysMemPitch = byteSize;
	initdata.SysMemSlicePitch = byteSize;

	ID3D11Buffer *releaseme = NULL;
	if(buf == NULL)
		buf = &releaseme;

	HRESULT hr = S_OK;
	
	CHECK_HR(dev->CreateBuffer(&bufDesc, data ? &initdata : NULL, buf));

	if(srv)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;

		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Format = fmt;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = byteSize/std::max(structSize, 1U);

		if(structSize == 0)
			desc.Buffer.NumElements = byteSize/16;

		CHECK_HR(dev->CreateShaderResourceView(*buf, &desc, srv));
	}

	if(uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Format = fmt;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.Flags = (type & BufUAVType) == eAppend ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		desc.Buffer.NumElements = byteSize/std::max(structSize, 1U);

		CHECK_HR(dev->CreateUnorderedAccessView(*buf, &desc, uav));
	}

	if(rtv)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;

		desc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
		desc.Format = fmt;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = byteSize/std::max(structSize, 1U);

		CHECK_HR(dev->CreateRenderTargetView(*buf, &desc, rtv));
	}

	if(releaseme) releaseme->Release();

	return 0;
}

int D3D11GraphicsTest::MakeTexture2D(UINT w, UINT h, UINT mips, DXGI_FORMAT fmt, ID3D11Texture2D **tex,
									ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav,
									ID3D11RenderTargetView **rtv, ID3D11DepthStencilView **dsv)
{
	return MakeTexture2DMS(w, h, mips, 1, fmt, tex, srv, uav, rtv, dsv);
}

int D3D11GraphicsTest::MakeTexture2DMS(UINT w, UINT h, UINT mips, UINT sampleCount, DXGI_FORMAT fmt, ID3D11Texture2D **tex,
									ID3D11ShaderResourceView **srv, ID3D11UnorderedAccessView **uav,
									ID3D11RenderTargetView **rtv, ID3D11DepthStencilView **dsv)
{
	D3D11_TEXTURE2D_DESC texdesc;

	texdesc.Width = w;
	texdesc.Height = h;
	texdesc.ArraySize = 1;
	texdesc.MipLevels = mips;
	texdesc.MiscFlags = 0;
	texdesc.CPUAccessFlags = 0;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = 0;
	texdesc.Format = fmt;
	texdesc.SampleDesc.Count = sampleCount;
	texdesc.SampleDesc.Quality = 0;

	if(srv)
		texdesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	if(rtv)
		texdesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if(dsv)
		texdesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	if(uav)
		texdesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	
	HRESULT hr = S_OK;
	
	CHECK_HR(dev->CreateTexture2D(&texdesc, NULL, tex));

	if(srv && srv != (ID3D11ShaderResourceView **)0x1)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		
		if(sampleCount > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			desc.Format = fmt;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Format = fmt;
			desc.Texture2D.MipLevels = 1;
			desc.Texture2D.MostDetailedMip = 0;
		}

		CHECK_HR(dev->CreateShaderResourceView(*tex, &desc, srv));
	}

	if(uav && sampleCount == 1)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;

		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Format = fmt;
		desc.Texture2D.MipSlice = 0;

		CHECK_HR(dev->CreateUnorderedAccessView(*tex, &desc, uav));
	}

	if(rtv && rtv != (ID3D11RenderTargetView **)0x1)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;

		if(sampleCount > 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			desc.Format = fmt;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			desc.Format = fmt;
			desc.Texture2D.MipSlice = 0;
		}

		CHECK_HR(dev->CreateRenderTargetView(*tex, &desc, rtv));
	}

	if(dsv && dsv != (ID3D11DepthStencilView **)0x1)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		
		if(sampleCount > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			desc.Flags = 0;
			desc.Format = fmt;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			desc.Flags = 0;
			desc.Format = fmt;
			desc.Texture2D.MipSlice = 0;
		}

		CHECK_HR(dev->CreateDepthStencilView(*tex, &desc, dsv));
	}

	return 0;
}

bool D3D11GraphicsTest::Running()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	// Check to see if any messages are waiting in the queue
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// Translate the message and dispatch it to WindowProc()
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// If the message is WM_QUIT, exit the program
	if (msg.message == WM_QUIT) return false;

	Sleep(20);

	return true;
}

void D3D11GraphicsTest::Present()
{
	swap->Present(0, 0);
}
