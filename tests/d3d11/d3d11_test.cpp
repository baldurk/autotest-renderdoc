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

#define INITGUID

#include "d3d11_test.h"
#include <stdio.h>
#include "../lz4.h"
#include "../renderdoc_app.h"
#include "../win32_window.h"

bool D3D11GraphicsTest::Init(int argc, char **argv)
{
  // parse parameters here to override parameters
  GraphicsTest::Init(argc, argv);

  // D3D11 specific can go here
  // ...

  HMODULE d3d11 = LoadLibraryA("d3d11.dll");
  HMODULE d3dcompiler = LoadLibraryA("d3dcompiler_47.dll");
  if(!d3dcompiler)
    d3dcompiler = LoadLibraryA("d3dcompiler_46.dll");
  if(!d3dcompiler)
    d3dcompiler = LoadLibraryA("d3dcompiler_45.dll");
  if(!d3dcompiler)
    d3dcompiler = LoadLibraryA("d3dcompiler_44.dll");
  if(!d3dcompiler)
    d3dcompiler = LoadLibraryA("d3dcompiler_43.dll");

  if(!d3d11 || !d3dcompiler)
  {
    TEST_ERROR("Couldn't load D3D11");
    return false;
  }

  dyn_D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d11, "D3D11CreateDevice");
  dyn_D3D11CreateDeviceAndSwapChain = (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(
      d3d11, "D3D11CreateDeviceAndSwapChain");

  dyn_D3DCompile = (pD3DCompile)GetProcAddress(d3dcompiler, "D3DCompile");
  dyn_D3DStripShader = (pD3DStripShader)GetProcAddress(d3dcompiler, "D3DStripShader");
  dyn_D3DSetBlobPart = (pD3DSetBlobPart)GetProcAddress(d3dcompiler, "D3DSetBlobPart");

  D3D_FEATURE_LEVEL features[] = {D3D_FEATURE_LEVEL_11_0};
  D3D_DRIVER_TYPE driver = D3D_DRIVER_TYPE_HARDWARE;

  if(d3d11_1)
    features[0] = D3D_FEATURE_LEVEL_11_1;

  HRESULT hr = S_OK;

  if(headless)
  {
    hr = dyn_D3D11CreateDevice(NULL, driver, NULL, debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0,
                               features, 1, D3D11_SDK_VERSION, &dev, NULL, &ctx);

    // if it failed but on a high feature level, try again on warp
    if(FAILED(hr) && features[0] != D3D_FEATURE_LEVEL_11_0)
    {
      driver = D3D_DRIVER_TYPE_WARP;
      hr = dyn_D3D11CreateDevice(NULL, driver, NULL, debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0,
                                 features, 1, D3D11_SDK_VERSION, &dev, NULL, &ctx);
    }

    // if it failed again on a high feature level, try last on ref
    if(FAILED(hr) && features[0] != D3D_FEATURE_LEVEL_11_0)
    {
      driver = D3D_DRIVER_TYPE_REFERENCE;
      hr = dyn_D3D11CreateDevice(NULL, driver, NULL, debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0,
                                 features, 1, D3D11_SDK_VERSION, &dev, NULL, &ctx);
    }

    if(FAILED(hr))
    {
      TEST_ERROR("D3D11CreateDevice failed: %x", hr);
      return false;
    }

    PostDeviceCreate();
    return true;
  }

  Win32Window *win = new Win32Window(screenWidth, screenHeight, screenTitle);

  window = win;

  DXGI_SWAP_CHAIN_DESC swapDesc;
  ZeroMemory(&swapDesc, sizeof(swapDesc));

  swapDesc.BufferCount = backbufferCount;
  swapDesc.BufferDesc.Format = backbufferFmt;
  swapDesc.BufferDesc.Width = screenWidth;
  swapDesc.BufferDesc.Height = screenHeight;
  swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
  swapDesc.SampleDesc.Count = backbufferMSAA;
  swapDesc.SampleDesc.Quality = 0;
  swapDesc.OutputWindow = win->wnd;
  swapDesc.Windowed = fullscreen ? FALSE : TRUE;
  swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapDesc.Flags = 0;

  hr = dyn_D3D11CreateDeviceAndSwapChain(NULL, driver, NULL,
                                         debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0, features, 1,
                                         D3D11_SDK_VERSION, &swapDesc, &swap, &dev, NULL, &ctx);

  // if it failed but on a high feature level, try again on warp
  if(FAILED(hr))
  {
    driver = D3D_DRIVER_TYPE_WARP;
    hr = dyn_D3D11CreateDeviceAndSwapChain(NULL, driver, NULL,
                                           debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0, features, 1,
                                           D3D11_SDK_VERSION, &swapDesc, &swap, &dev, NULL, &ctx);
  }

  // if it failed again on a high feature level, try last on ref
  if(FAILED(hr))
  {
    driver = D3D_DRIVER_TYPE_REFERENCE;
    hr = dyn_D3D11CreateDeviceAndSwapChain(NULL, driver, NULL,
                                           debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0, features, 1,
                                           D3D11_SDK_VERSION, &swapDesc, &swap, &dev, NULL, &ctx);
  }

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

  hr = dev->CreateRenderTargetView(bbTex, NULL, &bbRTV);

  if(FAILED(hr))
  {
    TEST_ERROR("CreateRenderTargetView failed: %x", hr);
    return false;
  }

  PostDeviceCreate();

  return true;
}

Window *D3D11GraphicsTest::MakeWindow(int width, int height, const char *title)
{
  return new Win32Window(width, height, title);
}

void D3D11GraphicsTest::PostDeviceCreate()
{
  // if(d3d11_1)
  {
    dev->QueryInterface(__uuidof(ID3D11Device1), (void **)&dev1);
    ctx->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&ctx1);
  }

  // if(d3d11_2)
  {
    dev->QueryInterface(__uuidof(ID3D11Device2), (void **)&dev2);
    ctx->QueryInterface(__uuidof(ID3D11DeviceContext2), (void **)&ctx2);
  }

  ctx->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&annot);

  dev->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, &opts, sizeof(opts));

  memset(&opts1, 0, sizeof(opts1));
  if(dev1)
    dev1->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &opts1, sizeof(opts1));
}

D3D11GraphicsTest::~D3D11GraphicsTest()
{
  delete window;
}

bool D3D11GraphicsTest::Running()
{
  return window->Update();
}

void D3D11GraphicsTest::Present()
{
  swap->Present(0, 0);
}

std::vector<byte> D3D11GraphicsTest::GetBufferData(ID3D11Buffer *buffer, uint32_t offset, uint32_t len)
{
  D3D11_MAPPED_SUBRESOURCE mapped;

  HRESULT hr = S_OK;

  TEST_ASSERT(buffer, "buffer is NULL");

  D3D11_BUFFER_DESC desc;
  buffer->GetDesc(&desc);

  if(len == 0)
    len = desc.ByteWidth - offset;

  if(len > 0 && offset + len > desc.ByteWidth)
  {
    TEST_WARN("Attempting to read off the end of the array. Will be clamped");
    len = std::min(len, desc.ByteWidth - offset);
  }

  ID3D11BufferPtr stage;

  desc.BindFlags = 0;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  CHECK_HR(dev->CreateBuffer(&desc, NULL, &stage));

  std::vector<byte> ret;
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

void D3D11GraphicsTest::IASetVertexBuffer(ID3D11Buffer *vb, UINT stride, UINT offset)
{
  ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
}

void D3D11GraphicsTest::ClearRenderTargetView(ID3D11RenderTargetView *rt, Vec4f col)
{
  ctx->ClearRenderTargetView(rt, &col.x);
}

void D3D11GraphicsTest::RSSetViewport(D3D11_VIEWPORT view)
{
  ctx->RSSetViewports(1, &view);
}

ID3DBlobPtr D3D11GraphicsTest::Compile(std::string src, std::string entry, std::string profile,
                                       ID3DBlob **unstripped)
{
  ID3DBlobPtr blob = NULL;
  ID3DBlobPtr error = NULL;

  HRESULT hr =
      dyn_D3DCompile(src.c_str(), src.length(), "", NULL, NULL, entry.c_str(), profile.c_str(),
                     D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG |
                         D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_OPTIMIZATION_LEVEL0,
                     0, &blob, &error);

  if(FAILED(hr))
  {
    TEST_ERROR("Failed to compile shader, error %x / %s", hr,
               error ? (char *)error->GetBufferPointer() : "Unknown");

    blob = NULL;
    error = NULL;
    return NULL;
  }

  if(unstripped)
  {
    blob.AddRef();
    *unstripped = blob.GetInterfacePtr();

    ID3DBlobPtr stripped = NULL;

    dyn_D3DStripShader(blob->GetBufferPointer(), blob->GetBufferSize(),
                       D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO, &stripped);

    blob = NULL;

    return stripped;
  }

  return blob;
}

void D3D11GraphicsTest::WriteBlob(std::string name, ID3DBlob *blob, bool compress)
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

    int compressedSize = LZ4_compress_default((const char *)blob->GetBufferPointer(), compBuf,
                                              uncompSize, uncompSize);

    fwrite(compBuf, 1, compressedSize, f);

    delete[] compBuf;
  }
  else
  {
    fwrite(blob->GetBufferPointer(), 1, blob->GetBufferSize(), f);
  }

  fclose(f);
}

ID3DBlobPtr D3D11GraphicsTest::SetBlobPath(std::string name, ID3DBlob *blob)
{
  ID3DBlobPtr newBlob = NULL;

  const GUID RENDERDOC_ShaderDebugMagicValue = RENDERDOC_ShaderDebugMagicValue_struct;

  std::string pathData;
  for(size_t i = 0; i < sizeof(RENDERDOC_ShaderDebugMagicValue); i++)
    pathData.push_back(' ');

  pathData += name;

  memcpy(&pathData[0], &RENDERDOC_ShaderDebugMagicValue, sizeof(RENDERDOC_ShaderDebugMagicValue));

  dyn_D3DSetBlobPart(blob->GetBufferPointer(), blob->GetBufferSize(), D3D_BLOB_PRIVATE_DATA, 0,
                     pathData.c_str(), pathData.size() + 1, &newBlob);

  return newBlob;
}

void D3D11GraphicsTest::SetBlobPath(std::string name, ID3D11DeviceChild *shader)
{
  const GUID RENDERDOC_ShaderDebugMagicValue = RENDERDOC_ShaderDebugMagicValue_struct;

  shader->SetPrivateData(RENDERDOC_ShaderDebugMagicValue, (UINT)name.size() + 1, name.c_str());
}

void D3D11GraphicsTest::CreateDefaultInputLayout(ID3DBlobPtr vsblob)
{
  D3D11_INPUT_ELEMENT_DESC layoutdesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  CHECK_HR(dev->CreateInputLayout(layoutdesc, ARRAY_COUNT(layoutdesc), vsblob->GetBufferPointer(),
                                  vsblob->GetBufferSize(), &defaultLayout));
}

ID3D11VertexShaderPtr D3D11GraphicsTest::CreateVS(ID3DBlobPtr blob)
{
  ID3D11VertexShaderPtr ret;
  CHECK_HR(dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &ret));
  return ret;
}

ID3D11PixelShaderPtr D3D11GraphicsTest::CreatePS(ID3DBlobPtr blob)
{
  ID3D11PixelShaderPtr ret;
  CHECK_HR(dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &ret));
  return ret;
}

ID3D11ComputeShaderPtr D3D11GraphicsTest::CreateCS(ID3DBlobPtr blob)
{
  ID3D11ComputeShaderPtr ret;
  CHECK_HR(dev->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &ret));
  return ret;
}

ID3D11GeometryShaderPtr D3D11GraphicsTest::CreateGS(ID3DBlobPtr blob,
                                                    const std::vector<D3D11_SO_DECLARATION_ENTRY> &sodecl,
                                                    const std::vector<UINT> &strides)
{
  ID3D11GeometryShaderPtr ret;
  CHECK_HR(dev->CreateGeometryShaderWithStreamOutput(blob->GetBufferPointer(), blob->GetBufferSize(),
                                                     &sodecl[0], (UINT)sodecl.size(), &strides[0],
                                                     (UINT)strides.size(), 0, NULL, &ret));
  return ret;
}
