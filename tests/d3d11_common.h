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

#define COM_SMARTPTR(classname) _COM_SMARTPTR_TYPEDEF(classname, __uuidof(classname))

COM_SMARTPTR(ID3DBlob);
COM_SMARTPTR(IDXGISwapChain);

COM_SMARTPTR(ID3D11Device);
COM_SMARTPTR(ID3D11Device1);
COM_SMARTPTR(ID3D11Device2);

COM_SMARTPTR(ID3D11DeviceContext);
COM_SMARTPTR(ID3D11DeviceContext1);
COM_SMARTPTR(ID3D11DeviceContext2);

COM_SMARTPTR(ID3D11VertexShader);
COM_SMARTPTR(ID3D11PixelShader);

struct D3D11GraphicsTest : public GraphicsTest
{
	D3D11GraphicsTest()
		: backbufferFmt(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
		  backbufferCount(2),
			backbufferMSAA(1),
		  d3d11_1(false)
	{
	}

	bool Init(int argc, char **argv);

	ID3DBlobPtr Compile(string src, string entry, string profile);

	bool Running();
	void Present();

	DXGI_FORMAT backbufferFmt;
	int backbufferCount;
	int backbufferMSAA;
	bool d3d11_1;

	IDXGISwapChainPtr swap;

	ID3D11DevicePtr dev;
	ID3D11Device1Ptr dev1;
	ID3D11Device2Ptr dev2;

	ID3D11DeviceContextPtr ctx;
	ID3D11DeviceContext1Ptr ctx1;
	ID3D11DeviceContext2Ptr ctx2;
};