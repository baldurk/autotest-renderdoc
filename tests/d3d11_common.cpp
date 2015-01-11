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

#include <stdio.h>

bool D3D11GraphicsTest::Init(int argc, char **argv)
{
	// parse parameters here to override parameters
	GraphicsTest::Init(argc, argv);

	// D3D11 specific can go here
	// ...

	HRESULT hr = S_OK;
	
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	
	swapDesc.BufferCount = backbufferCount;
	swapDesc.BufferDesc.Format = backbufferFmt;
	swapDesc.BufferDesc.Width = screenWidth;
	swapDesc.BufferDesc.Height = screenHeight;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.SampleDesc.Count = backbufferMSAA;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.OutputWindow = NULL; // hwnd
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
	
	TEST_UNIMPLEMENTED("D3D11GraphicsTest::Init");
	
	return false;
}

ID3DBlobPtr D3D11GraphicsTest::Compile(string src, string entry, string profile)
{
	TEST_UNIMPLEMENTED("D3D11GraphicsTest::Compile");

	return NULL;
}

bool D3D11GraphicsTest::Running()
{
	TEST_UNIMPLEMENTED("D3D11GraphicsTest::Running");

	return false;
}

void D3D11GraphicsTest::Present()
{
	TEST_UNIMPLEMENTED("D3D11GraphicsTest::Present");
}
