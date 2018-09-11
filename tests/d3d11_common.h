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

#include "test_common.h"

#include <comdef.h>
#include <windows.h>

#include <dxgi.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d9.h>

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

COM_SMARTPTR(ID3D11Query);
COM_SMARTPTR(ID3D11Counter);
COM_SMARTPTR(ID3D11Predicate);

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

COM_SMARTPTR(ID3D11InfoQueue);
COM_SMARTPTR(ID3DUserDefinedAnnotation);

extern std::string FullscreenQuadVertex;
extern std::string DefaultVertex;
extern std::string DefaultPixel;

#define GET_REFCOUNT(val, obj) \
  do                           \
  {                            \
    obj->AddRef();             \
    val = obj->Release();      \
  } while(0)

#define CHECK_HR(expr)                                                                    \
  {                                                                                       \
    HRESULT hr = (expr);                                                                  \
    if(FAILED(hr))                                                                        \
    {                                                                                     \
      TEST_ERROR("Failed HRESULT at %s:%d (%x): %s", __FILE__, (int)__LINE__, hr, #expr); \
      DEBUG_BREAK();                                                                      \
      exit(1);                                                                            \
    }                                                                                     \
  }

template <class T>
inline void SetDebugName(T pObj, const char *name)
{
  if(pObj)
    pObj->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
}

struct D3D11GraphicsTest;

class BufferCreator
{
public:
  BufferCreator(D3D11GraphicsTest *test);

  BufferCreator &Vertex();
  BufferCreator &Index();
  BufferCreator &Constant();
  BufferCreator &StreamOut();
  BufferCreator &SRV();
  BufferCreator &UAV();

  BufferCreator &Structured(UINT structStride);
  BufferCreator &ByteAddressed();
  BufferCreator &Mappable();
  BufferCreator &Staging();

  BufferCreator &Data(const void *data);
  BufferCreator &Size(UINT size);

  template <typename T, size_t N>
  BufferCreator &Data(const T (&data)[N])
  {
    return Data(&data[0]).Size(UINT(N * sizeof(T)));
  }

  template <typename T>
  BufferCreator &Data(const std::vector<T> &data)
  {
    return Data(data.data()).Size(UINT(data.size() * sizeof(T)));
  }

  operator ID3D11Buffer *() const;
  operator ID3D11BufferPtr() const { return ID3D11BufferPtr((ID3D11Buffer *)*this); }
private:
  D3D11GraphicsTest *m_Test;

  D3D11_BUFFER_DESC m_BufDesc;
  D3D11_SUBRESOURCE_DATA m_Initdata = {};
};

class TextureCreator
{
public:
  TextureCreator(D3D11GraphicsTest *test, DXGI_FORMAT format, UINT width, UINT height, UINT depth);

  TextureCreator &Mips(UINT mips);
  TextureCreator &Array(UINT size);
  TextureCreator &Multisampled(UINT count, UINT quality = 0);

  TextureCreator &SRV();
  TextureCreator &UAV();
  TextureCreator &RTV();
  TextureCreator &DSV();

  TextureCreator &Mappable();
  TextureCreator &Staging();

  operator ID3D11Texture1D *() const;
  operator ID3D11Texture1DPtr() const { return ID3D11Texture1DPtr((ID3D11Texture1D *)*this); }
  operator ID3D11Texture2D *() const;
  operator ID3D11Texture2DPtr() const { return ID3D11Texture2DPtr((ID3D11Texture2D *)*this); }
  operator ID3D11Texture3D *() const;
  operator ID3D11Texture3DPtr() const { return ID3D11Texture3DPtr((ID3D11Texture3D *)*this); }
protected:
  D3D11GraphicsTest *m_Test;

  UINT Width = 1;
  UINT Height = 1;
  UINT Depth = 1;
  UINT MipLevels = 1;
  UINT ArraySize = 1;
  DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
  DXGI_SAMPLE_DESC SampleDesc = {1, 0};
  D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
  UINT BindFlags = 0;
  UINT CPUAccessFlags = 0;
  UINT MiscFlags = 0;
};

enum class ResourceType
{
  Buffer,
  Texture1D,
  Texture1DArray,
  Texture2D,
  Texture2DArray,
  Texture2DMS,
  Texture2DMSArray,
  Texture3D,
};

enum class ViewType
{
  SRV,
  RTV,
  DSV,
  UAV,
};

class ViewCreator
{
public:
  ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Buffer *buf);
  ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture1D *tex);
  ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture2D *tex);
  ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture3D *tex);

  // common params
  ViewCreator &Format(DXGI_FORMAT format);

  // buffer params
  ViewCreator &FirstElement(UINT el);
  ViewCreator &NumElements(UINT num);

  // texture params
  ViewCreator &FirstMip(UINT mip);
  ViewCreator &NumMips(UINT num);
  ViewCreator &FirstSlice(UINT mip);
  ViewCreator &NumSlices(UINT num);

  // depth stencil only
  ViewCreator &ReadOnlyDepth();
  ViewCreator &ReadOnlyStencil();

  operator ID3D11ShaderResourceView *();
  operator ID3D11ShaderResourceViewPtr()
  {
    return ID3D11ShaderResourceViewPtr((ID3D11ShaderResourceView *)*this);
  }
  operator ID3D11RenderTargetView *();
  operator ID3D11RenderTargetViewPtr()
  {
    return ID3D11RenderTargetViewPtr((ID3D11RenderTargetView *)*this);
  }
  operator ID3D11DepthStencilView *();
  operator ID3D11DepthStencilViewPtr()
  {
    return ID3D11DepthStencilViewPtr((ID3D11DepthStencilView *)*this);
  }

  operator ID3D11UnorderedAccessView *();
  operator ID3D11UnorderedAccessViewPtr()
  {
    return ID3D11UnorderedAccessViewPtr((ID3D11UnorderedAccessView *)*this);
  }

private:
  void SetupDescriptors(ViewType viewType, ResourceType resType);

  D3D11GraphicsTest *m_Test;
  ID3D11Resource *m_Res;
  ViewType m_Type;

  // instead of a huge mess trying to auto populate the actual descriptors from saved values, as
  // they aren't very nicely compatible (e.g. RTVs have mipslice selection on 3D textures, SRVs
  // don't, SRVs support 1D texturse but DSVs don't, UAVs don't support MSAA textures, etc etc).
  // Instead we just set save pointers that might be NULL in the constructor based on view and
  // resource type
  union
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    D3D11_RENDER_TARGET_VIEW_DESC rtv;
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
  } desc;

  UINT *firstElement = NULL, *numElements = NULL;
  UINT *firstMip = NULL, *numMips = NULL;
  UINT *firstSlice = NULL, *numSlices = NULL;
};

class UAVCreator
{
public:
  UAVCreator(D3D11GraphicsTest *test, ID3D11Buffer *buf);
  UAVCreator(D3D11GraphicsTest *test, ID3D11Texture1D *tex);
  UAVCreator(D3D11GraphicsTest *test, ID3D11Texture2D *tex);
  UAVCreator(D3D11GraphicsTest *test, ID3D11Texture3D *tex);

  // common params
  UAVCreator &Format(DXGI_FORMAT format);

  // buffer params
  UAVCreator &FirstElement(UINT el);
  UAVCreator &NumElements(UINT num);

private:
  D3D11GraphicsTest *m_Test;

  ID3D11Resource *m_Res;
  D3D11_UNORDERED_ACCESS_VIEW_DESC m_Desc = {};
};

class RTVCreator
{
public:
  RTVCreator(D3D11GraphicsTest *test, ID3D11Texture1D *tex);
  RTVCreator(D3D11GraphicsTest *test, ID3D11Texture2D *tex);
  RTVCreator(D3D11GraphicsTest *test, ID3D11Texture3D *tex);

  // common params
  RTVCreator &Format(DXGI_FORMAT format);

private:
  D3D11GraphicsTest *m_Test;

  ID3D11Resource *m_Res;
  D3D11_RENDER_TARGET_VIEW_DESC m_Desc = {};
};

class DSVCreator
{
public:
  DSVCreator(D3D11GraphicsTest *test, ID3D11Texture1D *tex);
  DSVCreator(D3D11GraphicsTest *test, ID3D11Texture2D *tex);

  // common params
  DSVCreator &Format(DXGI_FORMAT format);

private:
  D3D11GraphicsTest *m_Test;

  ID3D11Resource *m_Res;
  D3D11_DEPTH_STENCIL_VIEW_DESC m_Desc = {};
};

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