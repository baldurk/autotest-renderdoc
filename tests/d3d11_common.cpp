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

std::string FullscreenQuadVertex = R"EOSHADER(

float4 main(uint vid : SV_VertexID) : SV_POSITION
{
	float2 positions[] = {
		float2(-1.0f,  1.0f),
		float2( 1.0f,  1.0f),
		float2(-1.0f, -1.0f),
		float2( 1.0f, -1.0f),
	};

	return float4(positions[vid], 0, 1);
}

)EOSHADER";

std::string DefaultVertex = R"EOSHADER(

struct vertin
{
	float3 pos : POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

struct v2f
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float4 uv : TEXCOORD0;
};

v2f main(vertin IN, uint vid : SV_VertexID)
{
	v2f OUT = (v2f)0;

	OUT.pos = float4(IN.pos.xyz, 1);
	OUT.col = IN.col;
	OUT.uv = float4(IN.uv, 0, 1);

	return OUT;
}

)EOSHADER";

std::string DefaultPixel = R"EOSHADER(

struct v2f
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float4 uv : TEXCOORD0;
};

float4 main(v2f IN) : SV_Target0
{
	return IN.col;
}

)EOSHADER";

const UINT formatStrides[] = {
    0,        // DXGI_FORMAT_UNKNOWN
    4 * 4,    // DXGI_FORMAT_R32G32B32A32_TYPELESS
    4 * 4,    // DXGI_FORMAT_R32G32B32A32_FLOAT
    4 * 4,    // DXGI_FORMAT_R32G32B32A32_UINT
    4 * 4,    // DXGI_FORMAT_R32G32B32A32_SINT
    4 * 3,    // DXGI_FORMAT_R32G32B32_TYPELESS
    4 * 3,    // DXGI_FORMAT_R32G32B32_FLOAT
    4 * 3,    // DXGI_FORMAT_R32G32B32_UINT
    4 * 3,    // DXGI_FORMAT_R32G32B32_SINT
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_TYPELESS
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_FLOAT
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_UNORM
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_UINT
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_SNORM
    2 * 4,    // DXGI_FORMAT_R16G16B16A16_SINT
    4 * 2,    // DXGI_FORMAT_R32G32_TYPELESS
    4 * 2,    // DXGI_FORMAT_R32G32_FLOAT
    4 * 2,    // DXGI_FORMAT_R32G32_UINT
    4 * 2,    // DXGI_FORMAT_R32G32_SINT
    4 * 2,    // DXGI_FORMAT_R32G8X24_TYPELESS
    5,        // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
    5,        // DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
    5,        // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
    4,        // DXGI_FORMAT_R10G10B10A2_TYPELESS
    4,        // DXGI_FORMAT_R10G10B10A2_UNORM
    4,        // DXGI_FORMAT_R10G10B10A2_UINT
    4,        // DXGI_FORMAT_R11G11B10_FLOAT
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_TYPELESS
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_UNORM
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_UINT
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_SNORM
    1 * 4,    // DXGI_FORMAT_R8G8B8A8_SINT
    2 * 2,    // DXGI_FORMAT_R16G16_TYPELESS
    2 * 2,    // DXGI_FORMAT_R16G16_FLOAT
    2 * 2,    // DXGI_FORMAT_R16G16_UNORM
    2 * 2,    // DXGI_FORMAT_R16G16_UINT
    2 * 2,    // DXGI_FORMAT_R16G16_SNORM
    2 * 2,    // DXGI_FORMAT_R16G16_SINT
    4 * 1,    // DXGI_FORMAT_R32_TYPELESS
    4 * 1,    // DXGI_FORMAT_D32_FLOAT
    4 * 1,    // DXGI_FORMAT_R32_FLOAT
    4 * 1,    // DXGI_FORMAT_R32_UINT
    4 * 1,    // DXGI_FORMAT_R32_SINT
    4,        // DXGI_FORMAT_R24G8_TYPELESS
    4,        // DXGI_FORMAT_D24_UNORM_S8_UINT
    4,        // DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    4,        // DXGI_FORMAT_X24_TYPELESS_G8_UINT
    1 * 2,    // DXGI_FORMAT_R8G8_TYPELESS
    1 * 2,    // DXGI_FORMAT_R8G8_UNORM
    1 * 2,    // DXGI_FORMAT_R8G8_UINT
    1 * 2,    // DXGI_FORMAT_R8G8_SNORM
    1 * 2,    // DXGI_FORMAT_R8G8_SINT
    2 * 1,    // DXGI_FORMAT_R16_TYPELESS
    2 * 1,    // DXGI_FORMAT_R16_FLOAT
    2,        // DXGI_FORMAT_D16_UNORM
    2 * 1,    // DXGI_FORMAT_R16_UNORM
    2 * 1,    // DXGI_FORMAT_R16_UINT
    2 * 1,    // DXGI_FORMAT_R16_SNORM
    2 * 1,    // DXGI_FORMAT_R16_SINT
    1 * 1,    // DXGI_FORMAT_R8_TYPELESS
    1 * 1,    // DXGI_FORMAT_R8_UNORM
    1 * 1,    // DXGI_FORMAT_R8_UINT
    1 * 1,    // DXGI_FORMAT_R8_SNORM
    1 * 1,    // DXGI_FORMAT_R8_SINT
    1,        // DXGI_FORMAT_A8_UNORM
    1,        // DXGI_FORMAT_R1_UNORM
    4,        // DXGI_FORMAT_R9G9B9E5_SHAREDEXP
    1 * 3,    // DXGI_FORMAT_R8G8_B8G8_UNORM
    1 * 3,    // DXGI_FORMAT_G8R8_G8B8_UNORM
    0,        // DXGI_FORMAT_BC1_TYPELESS
    0,        // DXGI_FORMAT_BC1_UNORM
    0,        // DXGI_FORMAT_BC1_UNORM_SRGB
    0,        // DXGI_FORMAT_BC2_TYPELESS
    0,        // DXGI_FORMAT_BC2_UNORM
    0,        // DXGI_FORMAT_BC2_UNORM_SRGB
    0,        // DXGI_FORMAT_BC3_TYPELESS
    0,        // DXGI_FORMAT_BC3_UNORM
    0,        // DXGI_FORMAT_BC3_UNORM_SRGB
    0,        // DXGI_FORMAT_BC4_TYPELESS
    0,        // DXGI_FORMAT_BC4_UNORM
    0,        // DXGI_FORMAT_BC4_SNORM
    0,        // DXGI_FORMAT_BC5_TYPELESS
    0,        // DXGI_FORMAT_BC5_UNORM
    0,        // DXGI_FORMAT_BC5_SNORM
    0,        // DXGI_FORMAT_B5G6R5_UNORM
    0,        // DXGI_FORMAT_B5G5R5A1_UNORM
    1 * 4,    // DXGI_FORMAT_B8G8R8A8_UNORM
    1 * 3,    // DXGI_FORMAT_B8G8R8X8_UNORM
    4,        // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
    1 * 4,    // DXGI_FORMAT_B8G8R8A8_TYPELESS
    1 * 4,    // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
    1 * 3,    // DXGI_FORMAT_B8G8R8X8_TYPELESS
    1 * 3,    // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
    0,        // DXGI_FORMAT_BC6H_TYPELESS
    0,        // DXGI_FORMAT_BC6H_UF16
    0,        // DXGI_FORMAT_BC6H_SF16
    0,        // DXGI_FORMAT_BC7_TYPELESS
    0,        // DXGI_FORMAT_BC7_UNORM
    0,        // DXGI_FORMAT_BC7_UNORM_SRGB
    0,        // DXGI_FORMAT_AYUV
    0,        // DXGI_FORMAT_Y410
    0,        // DXGI_FORMAT_Y416
    0,        // DXGI_FORMAT_NV12
    0,        // DXGI_FORMAT_P010
    0,        // DXGI_FORMAT_P016
    0,        // DXGI_FORMAT_420_OPAQUE
    0,        // DXGI_FORMAT_YUY2
    0,        // DXGI_FORMAT_Y210
    0,        // DXGI_FORMAT_Y216
    0,        // DXGI_FORMAT_NV11
    0,        // DXGI_FORMAT_AI44
    0,        // DXGI_FORMAT_IA44
    0,        // DXGI_FORMAT_P8
    0,        // DXGI_FORMAT_A8P8
    0,        // DXGI_FORMAT_B4G4R4A4_UNORM
};

#define RENDERDOC_ShaderDebugMagicValue_struct                                \
  {                                                                           \
    0xeab25520, 0x6670, 0x4865, 0x84, 0x29, 0x6c, 0x8, 0x51, 0x54, 0x00, 0xff \
  }

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if(msg == WM_CLOSE)
  {
    DestroyWindow(hwnd);
    return 0;
  }
  if(msg == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

typedef HRESULT(WINAPI *pD3DStripShader)(_In_reads_bytes_(BytecodeLength) LPCVOID pShaderBytecode,
                                         _In_ SIZE_T BytecodeLength, _In_ UINT uStripFlags,
                                         _Out_ ID3DBlob **ppStrippedBlob);
typedef HRESULT(WINAPI *pD3DSetBlobPart)(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
                                         _In_ SIZE_T SrcDataSize, _In_ D3D_BLOB_PART Part,
                                         _In_ UINT Flags, _In_reads_bytes_(PartSize) LPCVOID pPart,
                                         _In_ SIZE_T PartSize, _Out_ ID3DBlob **ppNewShader);

pD3DCompile dyn_D3DCompile = NULL;
pD3DStripShader dyn_D3DStripShader = NULL;
pD3DSetBlobPart dyn_D3DSetBlobPart = NULL;

PFN_D3D11_CREATE_DEVICE dyn_D3D11CreateDevice = NULL;
PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN dyn_D3D11CreateDeviceAndSwapChain = NULL;

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

  string classname = "renderdoc_d3d11_test";
  static int idx = 0;
  idx++;
  classname += '0' + idx;

  WNDCLASSEXA wc;
  wc.cbSize = sizeof(WNDCLASSEXA);
  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = classname.c_str();
  wc.hIconSm = NULL;

  if(!RegisterClassEx(&wc))
  {
    return false;
  }

  RECT rect = {0, 0, screenWidth, screenHeight};
  AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

  wnd = CreateWindowExA(WS_EX_CLIENTEDGE, classname.c_str(), "RenderDoc test program",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
                        rect.bottom - rect.top, NULL, NULL, NULL, NULL);

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

  ShowWindow(wnd, SW_SHOW);
  UpdateWindow(wnd);

  PostDeviceCreate();

  return true;
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
}

D3D11GraphicsTest::~D3D11GraphicsTest()
{
  if(wnd)
    DestroyWindow(wnd);
}

ID3DBlobPtr D3D11GraphicsTest::Compile(string src, string entry, string profile, ID3DBlob **unstripped)
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

ID3DBlobPtr D3D11GraphicsTest::SetBlobPath(string name, ID3DBlob *blob)
{
  ID3DBlobPtr newBlob = NULL;

  const GUID RENDERDOC_ShaderDebugMagicValue = RENDERDOC_ShaderDebugMagicValue_struct;

  string pathData;
  for(size_t i = 0; i < sizeof(RENDERDOC_ShaderDebugMagicValue); i++)
    pathData.push_back(' ');

  pathData += name;

  memcpy(&pathData[0], &RENDERDOC_ShaderDebugMagicValue, sizeof(RENDERDOC_ShaderDebugMagicValue));

  dyn_D3DSetBlobPart(blob->GetBufferPointer(), blob->GetBufferSize(), D3D_BLOB_PRIVATE_DATA, 0,
                     pathData.c_str(), pathData.size() + 1, &newBlob);

  return newBlob;
}

void D3D11GraphicsTest::SetBlobPath(string name, ID3D11DeviceChild *shader)
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

vector<byte> D3D11GraphicsTest::GetBufferData(ID3D11Buffer *buffer, uint32_t offset, uint32_t len)
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

bool D3D11GraphicsTest::Running()
{
  MSG msg;
  ZeroMemory(&msg, sizeof(msg));

  // Check to see if any messages are waiting in the queue
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    // Translate the message and dispatch it to WindowProc()
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // If the message is WM_QUIT, exit the program
  if(msg.message == WM_QUIT)
    return false;

  if(msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
    return false;

  Sleep(20);

  return true;
}

void D3D11GraphicsTest::Present()
{
  swap->Present(0, 0);
}

BufferCreator::BufferCreator(D3D11GraphicsTest *test) : m_Test(test)
{
  m_BufDesc.ByteWidth = 0;
  m_BufDesc.MiscFlags = 0;
  m_BufDesc.StructureByteStride = 0;
  m_BufDesc.CPUAccessFlags = 0;
  m_BufDesc.Usage = D3D11_USAGE_DEFAULT;
  m_BufDesc.BindFlags = 0;
}

BufferCreator &BufferCreator::Vertex()
{
  m_BufDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
  return *this;
}

BufferCreator &BufferCreator::Index()
{
  m_BufDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
  return *this;
}

BufferCreator &BufferCreator::Constant()
{
  m_BufDesc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
  return *this;
}

BufferCreator &BufferCreator::StreamOut()
{
  m_BufDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
  return *this;
}

BufferCreator &BufferCreator::SRV()
{
  m_BufDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
  return *this;
}

BufferCreator &BufferCreator::UAV()
{
  m_BufDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
  return *this;
}

BufferCreator &BufferCreator::Structured(UINT structStride)
{
  if(structStride > 0 && (m_BufDesc.ByteWidth % structStride) != 0)
    TEST_FATAL("Invalid structure size - not divisor of byte size");

  m_BufDesc.StructureByteStride = structStride;
  m_BufDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  return *this;
}

BufferCreator &BufferCreator::ByteAddressed()
{
  m_BufDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
  return *this;
}

BufferCreator &BufferCreator::Mappable()
{
  m_BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  m_BufDesc.Usage = D3D11_USAGE_DYNAMIC;
  return *this;
}

BufferCreator &BufferCreator::Staging()
{
  m_BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  m_BufDesc.Usage = D3D11_USAGE_STAGING;
  return *this;
}

BufferCreator &BufferCreator::Data(const void *data)
{
  m_Initdata.pSysMem = data;
  m_Initdata.SysMemPitch = m_BufDesc.ByteWidth;
  m_Initdata.SysMemSlicePitch = m_BufDesc.ByteWidth;
  return *this;
}

BufferCreator &BufferCreator::Size(UINT size)
{
  if(m_BufDesc.BindFlags & D3D11_BIND_CONSTANT_BUFFER)
    size = (size + 15) & ~0xf;

  m_BufDesc.ByteWidth = size;

  m_Initdata.SysMemPitch = size;
  m_Initdata.SysMemSlicePitch = size;
  return *this;
}

BufferCreator::operator ID3D11Buffer *() const
{
  ID3D11Buffer *buf = NULL;
  CHECK_HR(m_Test->dev->CreateBuffer(&m_BufDesc, m_Initdata.pSysMem ? &m_Initdata : NULL, &buf));
  return buf;
}

TextureCreator::TextureCreator(D3D11GraphicsTest *test, DXGI_FORMAT format, UINT width, UINT height,
                               UINT depth)
    : m_Test(test)
{
  Format = format;
  Width = width;
  Height = height;
  depth = depth;
}

TextureCreator &TextureCreator::Mips(UINT mips)
{
  MipLevels = mips;
  return *this;
}

TextureCreator &TextureCreator::Array(UINT size)
{
  ArraySize = size;
  return *this;
}

TextureCreator &TextureCreator::Multisampled(UINT count, UINT quality)
{
  SampleDesc.Count = count;
  SampleDesc.Quality = quality;
  return *this;
}

TextureCreator &TextureCreator::SRV()
{
  BindFlags |= D3D11_BIND_SHADER_RESOURCE;
  return *this;
}

TextureCreator &TextureCreator::UAV()
{
  BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
  return *this;
}

TextureCreator &TextureCreator::RTV()
{
  BindFlags |= D3D11_BIND_RENDER_TARGET;
  return *this;
}

TextureCreator &TextureCreator::DSV()
{
  BindFlags |= D3D11_BIND_DEPTH_STENCIL;
  return *this;
}

TextureCreator &TextureCreator::Mappable()
{
  CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  Usage = D3D11_USAGE_DYNAMIC;
  return *this;
}

TextureCreator &TextureCreator::Staging()
{
  CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  Usage = D3D11_USAGE_STAGING;
  return *this;
}

TextureCreator::operator ID3D11Texture1D *() const
{
  D3D11_TEXTURE1D_DESC texdesc;

  texdesc.Width = Width;
  texdesc.ArraySize = ArraySize;
  texdesc.MipLevels = MipLevels;
  texdesc.MiscFlags = MiscFlags;
  texdesc.CPUAccessFlags = CPUAccessFlags;
  texdesc.Usage = Usage;
  texdesc.BindFlags = BindFlags;
  texdesc.Format = Format;

  ID3D11Texture1D *tex = NULL;
  CHECK_HR(m_Test->dev->CreateTexture1D(&texdesc, NULL, &tex));
  return tex;
}

TextureCreator::operator ID3D11Texture2D *() const
{
  D3D11_TEXTURE2D_DESC texdesc;

  texdesc.Width = Width;
  texdesc.Height = Height;
  texdesc.ArraySize = ArraySize;
  texdesc.MipLevels = MipLevels;
  texdesc.MiscFlags = MiscFlags;
  texdesc.CPUAccessFlags = CPUAccessFlags;
  texdesc.Usage = Usage;
  texdesc.BindFlags = BindFlags;
  texdesc.Format = Format;
  texdesc.SampleDesc.Count = SampleDesc.Count;
  texdesc.SampleDesc.Quality = SampleDesc.Quality;

  ID3D11Texture2D *tex = NULL;
  CHECK_HR(m_Test->dev->CreateTexture2D(&texdesc, NULL, &tex));
  return tex;
}

TextureCreator::operator ID3D11Texture3D *() const
{
  D3D11_TEXTURE3D_DESC texdesc;

  texdesc.Width = Width;
  texdesc.Height = Height;
  texdesc.Depth = Height;
  texdesc.MipLevels = MipLevels;
  texdesc.MiscFlags = MiscFlags;
  texdesc.CPUAccessFlags = CPUAccessFlags;
  texdesc.Usage = Usage;
  texdesc.BindFlags = BindFlags;
  texdesc.Format = Format;

  ID3D11Texture3D *tex = NULL;
  CHECK_HR(m_Test->dev->CreateTexture3D(&texdesc, NULL, &tex));
  return tex;
}

ViewCreator::ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Buffer *buf)
    : m_Test(test), m_Type(viewType), m_Res(buf)
{
  SetupDescriptors(viewType, ResourceType::Buffer);
}

ViewCreator::ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture1D *tex)
    : m_Test(test), m_Type(viewType), m_Res(tex)
{
  D3D11_TEXTURE1D_DESC texdesc;
  tex->GetDesc(&texdesc);

  ResourceType resType = ResourceType::Texture1D;

  if(texdesc.ArraySize > 1)
    resType = ResourceType::Texture1DArray;

  SetupDescriptors(viewType, resType);

  Format(texdesc.Format);
}

ViewCreator::ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture2D *tex)
    : m_Test(test), m_Type(viewType), m_Res(tex)
{
  D3D11_TEXTURE2D_DESC texdesc;
  tex->GetDesc(&texdesc);

  ResourceType resType;

  if(texdesc.SampleDesc.Count > 1)
  {
    resType = ResourceType::Texture2DMS;
    if(texdesc.ArraySize > 1)
      resType = ResourceType::Texture2DMSArray;
  }
  else
  {
    resType = ResourceType::Texture2D;
    if(texdesc.ArraySize > 1)
      resType = ResourceType::Texture2DArray;
  }

  SetupDescriptors(viewType, resType);

  Format(texdesc.Format);
}

ViewCreator::ViewCreator(D3D11GraphicsTest *test, ViewType viewType, ID3D11Texture3D *tex)
    : m_Test(test), m_Type(viewType), m_Res(tex)
{
  D3D11_TEXTURE3D_DESC texdesc;
  tex->GetDesc(&texdesc);

  SetupDescriptors(viewType, ResourceType::Texture3D);

  Format(texdesc.Format);
}

void ViewCreator::SetupDescriptors(ViewType viewType, ResourceType resType)
{
  memset(&desc, 0, sizeof(desc));

  constexpr D3D11_SRV_DIMENSION srvDim[] = {
      D3D11_SRV_DIMENSION_BUFFER,              // Buffer
      D3D11_SRV_DIMENSION_TEXTURE1D,           // Texture1D
      D3D11_SRV_DIMENSION_TEXTURE1DARRAY,      // Texture1DArray
      D3D11_SRV_DIMENSION_TEXTURE2D,           // Texture2D
      D3D11_SRV_DIMENSION_TEXTURE2DARRAY,      // Texture2DArray
      D3D11_SRV_DIMENSION_TEXTURE2DMS,         // Texture2DMS
      D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY,    // Texture2DMSArray
      D3D11_SRV_DIMENSION_TEXTURE3D,           // Texture3D
  };

  constexpr D3D11_RTV_DIMENSION rtvDim[] = {
      D3D11_RTV_DIMENSION_BUFFER,              // Buffer
      D3D11_RTV_DIMENSION_TEXTURE1D,           // Texture1D
      D3D11_RTV_DIMENSION_TEXTURE1DARRAY,      // Texture1DArray
      D3D11_RTV_DIMENSION_TEXTURE2D,           // Texture2D
      D3D11_RTV_DIMENSION_TEXTURE2DARRAY,      // Texture2DArray
      D3D11_RTV_DIMENSION_TEXTURE2DMS,         // Texture2DMS
      D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY,    // Texture2DMSArray
      D3D11_RTV_DIMENSION_TEXTURE3D,           // Texture3D
  };

  constexpr D3D11_DSV_DIMENSION dsvDim[] = {
      D3D11_DSV_DIMENSION_UNKNOWN,             // Buffer
      D3D11_DSV_DIMENSION_TEXTURE1D,           // Texture1D
      D3D11_DSV_DIMENSION_TEXTURE1DARRAY,      // Texture1DArray
      D3D11_DSV_DIMENSION_TEXTURE2D,           // Texture2D
      D3D11_DSV_DIMENSION_TEXTURE2DARRAY,      // Texture2DArray
      D3D11_DSV_DIMENSION_TEXTURE2DMS,         // Texture2DMS
      D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY,    // Texture2DMSArray
      D3D11_DSV_DIMENSION_UNKNOWN,             // Texture3D
  };

  constexpr D3D11_UAV_DIMENSION uavDim[] = {
      D3D11_UAV_DIMENSION_BUFFER,            // Buffer
      D3D11_UAV_DIMENSION_TEXTURE1D,         // Texture1D
      D3D11_UAV_DIMENSION_TEXTURE1DARRAY,    // Texture1DArray
      D3D11_UAV_DIMENSION_TEXTURE2D,         // Texture2D
      D3D11_UAV_DIMENSION_TEXTURE2DARRAY,    // Texture2DArray
      D3D11_UAV_DIMENSION_UNKNOWN,           // Texture2DMS
      D3D11_UAV_DIMENSION_UNKNOWN,           // Texture2DMSArray
      D3D11_UAV_DIMENSION_TEXTURE3D,         // Texture3D
  };

  if(viewType == ViewType::SRV)
  {
    desc.srv.ViewDimension = srvDim[(int)resType];

    if(resType == ResourceType::Buffer)
    {
      firstElement = &desc.srv.Buffer.FirstElement;
      numElements = &desc.srv.Buffer.NumElements;
    }
  }
  else if(viewType == ViewType::RTV)
  {
    desc.rtv.ViewDimension = rtvDim[(int)resType];

    if(resType == ResourceType::Buffer)
    {
      firstElement = &desc.rtv.Buffer.FirstElement;
      numElements = &desc.rtv.Buffer.NumElements;
    }
  }
  else if(viewType == ViewType::DSV)
  {
    desc.dsv.ViewDimension = dsvDim[(int)resType];

    if(desc.dsv.ViewDimension == D3D11_DSV_DIMENSION_UNKNOWN)
      TEST_FATAL("Unsupported resource for DSV");
  }
  else if(viewType == ViewType::UAV)
  {
    desc.uav.ViewDimension = uavDim[(int)resType];

    if(desc.uav.ViewDimension == D3D11_UAV_DIMENSION_UNKNOWN)
      TEST_FATAL("Unsupported resource for UAV");

    if(resType == ResourceType::Buffer)
    {
      firstElement = &desc.uav.Buffer.FirstElement;
      numElements = &desc.uav.Buffer.NumElements;
    }
  }

  UINT *pointers[4][8][4] = {
      // SRV
      {
          // &firstMip, &numMips, &firstSlice, &numSlices
          {
              NULL, NULL, NULL, NULL,
          },
          {
              &desc.srv.Texture1D.MostDetailedMip, &desc.srv.Texture1D.MipLevels, NULL, NULL,
          },
          {
              &desc.srv.Texture1DArray.MostDetailedMip, &desc.srv.Texture1DArray.MipLevels,
              &desc.srv.Texture1DArray.FirstArraySlice, &desc.srv.Texture1DArray.ArraySize,
          },
          {
              &desc.srv.Texture2D.MostDetailedMip, &desc.srv.Texture2D.MipLevels, NULL, NULL,
          },
          {
              &desc.srv.Texture2DArray.MostDetailedMip, &desc.srv.Texture2DArray.MipLevels,
              &desc.srv.Texture2DArray.FirstArraySlice, &desc.srv.Texture2DArray.ArraySize,
          },
          {
              NULL, NULL, NULL, NULL,
          },
          {
              NULL, NULL, &desc.srv.Texture2DMSArray.FirstArraySlice,
              &desc.srv.Texture2DMSArray.ArraySize,
          },
          {
              &desc.srv.Texture3D.MostDetailedMip, &desc.srv.Texture2D.MipLevels, NULL, NULL,
          },
      },
      // RTV
      {
          // &firstMip, &numMips, &firstSlice, &numSlices
          {
              NULL, NULL, NULL, NULL,
          },
          {
              &desc.rtv.Texture1D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.rtv.Texture1DArray.MipSlice, NULL, &desc.rtv.Texture1DArray.FirstArraySlice,
              &desc.rtv.Texture1DArray.ArraySize,
          },
          {
              &desc.rtv.Texture2D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.rtv.Texture2DArray.MipSlice, NULL, &desc.rtv.Texture2DArray.FirstArraySlice,
              &desc.rtv.Texture2DArray.ArraySize,
          },
          {
              NULL, NULL, NULL, NULL,
          },
          {
              NULL, NULL, &desc.rtv.Texture2DMSArray.FirstArraySlice,
              &desc.rtv.Texture2DMSArray.ArraySize,
          },
          {
              &desc.rtv.Texture3D.MipSlice, NULL, &desc.rtv.Texture3D.FirstWSlice,
              &desc.rtv.Texture3D.WSize,
          },
      },
      // DSV
      {
          // &firstMip, &numMips, &firstSlice, &numSlices
          {
              NULL, NULL, NULL, NULL,
          },
          {
              &desc.dsv.Texture1D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.dsv.Texture1DArray.MipSlice, NULL, &desc.dsv.Texture1DArray.FirstArraySlice,
              &desc.dsv.Texture1DArray.ArraySize,
          },
          {
              &desc.dsv.Texture2D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.dsv.Texture2DArray.MipSlice, NULL, &desc.dsv.Texture2DArray.FirstArraySlice,
              &desc.dsv.Texture2DArray.ArraySize,
          },
          {
              NULL, NULL, NULL, NULL,
          },
          {
              NULL, NULL, &desc.dsv.Texture2DMSArray.FirstArraySlice,
              &desc.dsv.Texture2DMSArray.ArraySize,
          },
          {
              NULL, NULL, NULL, NULL,
          },
      },
      // UAV
      {
          // &firstMip, &numMips, &firstSlice, &numSlices
          {
              NULL, NULL, NULL, NULL,
          },
          {
              &desc.uav.Texture1D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.uav.Texture1DArray.MipSlice, NULL, &desc.uav.Texture1DArray.FirstArraySlice,
              &desc.uav.Texture1DArray.ArraySize,
          },
          {
              &desc.uav.Texture2D.MipSlice, NULL, NULL, NULL,
          },
          {
              &desc.uav.Texture2DArray.MipSlice, NULL, &desc.uav.Texture2DArray.FirstArraySlice,
              &desc.uav.Texture2DArray.ArraySize,
          },
          {
              NULL, NULL, NULL, NULL,
          },
          {
              NULL, NULL, NULL, NULL,
          },
          {
              &desc.uav.Texture3D.MipSlice, NULL, &desc.uav.Texture3D.FirstWSlice,
              &desc.uav.Texture3D.WSize,
          },
      },
  };

  if(resType != ResourceType::Buffer)
  {
    firstMip = pointers[(int)viewType][(int)resType][0];
    numMips = pointers[(int)viewType][(int)resType][1];
    firstSlice = pointers[(int)viewType][(int)resType][2];
    numSlices = pointers[(int)viewType][(int)resType][3];

    if(numMips)
      *numMips = ~0U;
    if(numSlices)
      *numSlices = ~0U;
  }
}

ViewCreator &ViewCreator::Format(DXGI_FORMAT f)
{
  // this is always in the same place, just write it once
  desc.srv.Format = f;
  return *this;
}

ViewCreator &ViewCreator::FirstElement(UINT el)
{
  if(firstElement)
    *firstElement = el;
  return *this;
}

ViewCreator &ViewCreator::NumElements(UINT num)
{
  if(numElements)
    *numElements = num;
  else
    TEST_ERROR("This view & resource doesn't support NumElements");
  return *this;
}

ViewCreator &ViewCreator::FirstMip(UINT mip)
{
  if(firstMip)
    *firstMip = mip;
  else
    TEST_ERROR("This view & resource doesn't support FirstMip");
  return *this;
}

ViewCreator &ViewCreator::NumMips(UINT num)
{
  if(numMips)
    *numMips = num;
  else
    TEST_ERROR("This view & resource doesn't support NumMips");
  return *this;
}

ViewCreator &ViewCreator::FirstSlice(UINT mip)
{
  if(firstSlice)
    *firstSlice = mip;
  else
    TEST_ERROR("This view & resource doesn't support FirstSlice");
  return *this;
}

ViewCreator &ViewCreator::NumSlices(UINT num)
{
  if(numSlices)
    *numSlices = num;
  else
    TEST_ERROR("This view & resource doesn't support NumSlices");
  return *this;
}

ViewCreator &ViewCreator::ReadOnlyDepth()
{
  desc.dsv.Flags |= D3D11_DSV_READ_ONLY_DEPTH;
  return *this;
}

ViewCreator &ViewCreator::ReadOnlyStencil()
{
  desc.dsv.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
  return *this;
}

ViewCreator::operator ID3D11ShaderResourceView *()
{
  if(desc.srv.ViewDimension == D3D11_SRV_DIMENSION_BUFFER)
  {
    D3D11_BUFFER_DESC bufdesc;
    ((ID3D11Buffer *)m_Res)->GetDesc(&bufdesc);

    if(bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
      desc.srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
      desc.srv.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    }

    UINT elementStride = bufdesc.StructureByteStride;

    if(bufdesc.StructureByteStride == 0 && desc.srv.Format == DXGI_FORMAT_UNKNOWN)
      TEST_FATAL("Can't create SRV on non-structured buffer with no format");

    if(desc.srv.Format != DXGI_FORMAT_UNKNOWN)
      elementStride = formatStrides[desc.srv.Format];

    if(desc.srv.Buffer.NumElements == 0)
      desc.srv.Buffer.NumElements = bufdesc.ByteWidth / std::max(elementStride, 1U);
  }

  TEST_ASSERT(m_Res, "Must have resource");
  TEST_ASSERT(m_Type == ViewType::SRV, "Casting non-SRV ViewCreator to SRV");

  ID3D11ShaderResourceView *srv = NULL;
  CHECK_HR(m_Test->dev->CreateShaderResourceView(m_Res, &desc.srv, &srv));
  return srv;
}

ViewCreator::operator ID3D11UnorderedAccessView *()
{
  if(desc.uav.ViewDimension == D3D11_UAV_DIMENSION_BUFFER)
  {
    D3D11_BUFFER_DESC bufdesc;
    ((ID3D11Buffer *)m_Res)->GetDesc(&bufdesc);

    if(bufdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
      desc.uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

    UINT elementStride = bufdesc.StructureByteStride;

    if(bufdesc.StructureByteStride == 0 && desc.uav.Format == DXGI_FORMAT_UNKNOWN)
      TEST_FATAL("Can't create uav on non-structured buffer with no format");

    if(desc.uav.Format != DXGI_FORMAT_UNKNOWN)
      elementStride = formatStrides[desc.uav.Format];

    if(desc.uav.Buffer.NumElements == 0)
      desc.uav.Buffer.NumElements = bufdesc.ByteWidth / std::max(elementStride, 1U);
  }

  TEST_ASSERT(m_Res, "Must have resource");
  TEST_ASSERT(m_Type == ViewType::UAV, "Casting non-UAV ViewCreator to UAV");

  ID3D11UnorderedAccessView *uav = NULL;
  CHECK_HR(m_Test->dev->CreateUnorderedAccessView(m_Res, &desc.uav, &uav));
  return uav;
}

ViewCreator::operator ID3D11RenderTargetView *()
{
  TEST_ASSERT(m_Res, "Must have resource");
  TEST_ASSERT(m_Type == ViewType::RTV, "Casting non-RTV ViewCreator to RTV");

  ID3D11RenderTargetView *rtv = NULL;
  CHECK_HR(m_Test->dev->CreateRenderTargetView(m_Res, &desc.rtv, &rtv));
  return rtv;
}

ViewCreator::operator ID3D11DepthStencilView *()
{
  TEST_ASSERT(m_Res, "Must have resource");
  TEST_ASSERT(m_Type == ViewType::DSV, "Casting non-DSV ViewCreator to DSV");

  ID3D11DepthStencilView *dsv = NULL;
  CHECK_HR(m_Test->dev->CreateDepthStencilView(m_Res, &desc.dsv, &dsv));
  return dsv;
}
