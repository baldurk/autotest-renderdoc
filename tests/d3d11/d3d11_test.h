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

#define INITGUID

#include "../test_common.h"

#include <comdef.h>
#include <windows.h>

#include <dxgi.h>

#include <d3dcompiler.h>
#include "d3d11_helpers.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

typedef HRESULT(WINAPI *pD3DStripShader)(_In_reads_bytes_(BytecodeLength) LPCVOID pShaderBytecode,
                                         _In_ SIZE_T BytecodeLength, _In_ UINT uStripFlags,
                                         _Out_ ID3DBlob **ppStrippedBlob);
typedef HRESULT(WINAPI *pD3DSetBlobPart)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
                                         _In_ SIZE_T SrcDataSize, _In_ D3D_BLOB_PART Part,
                                         _In_ UINT Flags, _In_reads_bytes_(PartSize) LPCVOID pPart,
                                         _In_ SIZE_T PartSize, _Out_ ID3DBlob **ppNewShader);

struct D3D11GraphicsTest : public GraphicsTest
{
  static const TestAPI API = TestAPI::D3D11;

  D3D11GraphicsTest()
      : backbufferFmt(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
        backbufferCount(2),
        backbufferMSAA(1),
        d3d11_1(false),
        d3d11_2(false),
        wnd(NULL)
  {
  }

  ~D3D11GraphicsTest();

  bool Init(int argc, char **argv);

  void PostDeviceCreate();

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
    eRawBuffer = 0x200,
    BufUAVType = 0xf00,
  };

  ID3DBlobPtr Compile(string src, string entry, string profile, ID3DBlob **unstripped = NULL);
  void WriteBlob(string name, ID3DBlob *blob, bool compress);

  ID3D11VertexShaderPtr CreateVS(ID3DBlobPtr blob);
  ID3D11PixelShaderPtr CreatePS(ID3DBlobPtr blob);
  ID3D11ComputeShaderPtr CreateCS(ID3DBlobPtr blob);
  ID3D11GeometryShaderPtr CreateGS(ID3DBlobPtr blob,
                                   const std::vector<D3D11_SO_DECLARATION_ENTRY> &sodecl,
                                   const std::vector<UINT> &strides);

  ID3DBlobPtr SetBlobPath(string name, ID3DBlob *blob);
  void SetBlobPath(string name, ID3D11DeviceChild *shader);

  void CreateDefaultInputLayout(ID3DBlobPtr vsblob);

  BufferCreator MakeBuffer() { return BufferCreator(this); }
  TextureCreator MakeTexture(DXGI_FORMAT format, UINT width)
  {
    return TextureCreator(this, format, width, 1, 1);
  }
  TextureCreator MakeTexture(DXGI_FORMAT format, UINT width, UINT height)
  {
    return TextureCreator(this, format, width, height, 1);
  }
  TextureCreator MakeTexture(DXGI_FORMAT format, UINT width, UINT height, UINT depth)
  {
    return TextureCreator(this, format, width, height, depth);
  }

  template <typename T>
  ViewCreator MakeSRV(T res)
  {
    return ViewCreator(this, ViewType::SRV, res);
  }
  template <typename T>
  ViewCreator MakeRTV(T res)
  {
    return ViewCreator(this, ViewType::RTV, res);
  }
  template <typename T>
  ViewCreator MakeDSV(T res)
  {
    return ViewCreator(this, ViewType::DSV, res);
  }
  template <typename T>
  ViewCreator MakeUAV(T res)
  {
    return ViewCreator(this, ViewType::UAV, res);
  }

  vector<byte> GetBufferData(ID3D11Buffer *buf, uint32_t offset = 0, uint32_t len = 0);

  D3D11_MAPPED_SUBRESOURCE Map(ID3D11Resource *res, UINT sub, D3D11_MAP type)
  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    ctx->Map(res, sub, type, 0, &mapped);
    return mapped;
  }

  struct VBBind
  {
    ID3D11Buffer *buf;
    UINT stride;
    UINT offset;
  };

  void IASetVertexBuffer(ID3D11Buffer *vb, UINT stride, UINT offset);

  void ClearRenderTargetView(ID3D11RenderTargetView *rt, Vec4f col);

  void RSSetViewport(D3D11_VIEWPORT view);

  bool Running();
  void Present();

  DXGI_FORMAT backbufferFmt;
  int backbufferCount;
  int backbufferMSAA;
  bool d3d11_1;
  bool d3d11_2;

  pD3DCompile dyn_D3DCompile = NULL;
  pD3DStripShader dyn_D3DStripShader = NULL;
  pD3DSetBlobPart dyn_D3DSetBlobPart = NULL;

  PFN_D3D11_CREATE_DEVICE dyn_D3D11CreateDevice = NULL;
  PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN dyn_D3D11CreateDeviceAndSwapChain = NULL;

  HWND wnd;

  IDXGISwapChainPtr swap;

  ID3D11InputLayoutPtr defaultLayout;

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