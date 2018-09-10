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
#include "../gl_common.h"

#include "../glad/glad_wgl.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <GLFW/glfw3native.h>

namespace
{
string dxcommon = R"EOSHADER(

struct v2f
{
  float4 pos : SV_Position;
  float2 uv : UV;
};

)EOSHADER";

string dxvertex = R"EOSHADER(

v2f main(uint vid : SV_VertexID)
{
	float2 positions[] = {
		float2(-1.0f, -1.0f),
		float2(-1.0f,  1.0f),
		float2( 1.0f, -1.0f),
		float2( 1.0f,  1.0f),
	};

  v2f OUT = (v2f)0;

	OUT.pos = float4(positions[vid]*0.8f, 0, 1);
  OUT.uv = positions[vid]*0.5f + 0.5f;

  return OUT;
}

)EOSHADER";

string dxpixel = R"EOSHADER(

Texture2D<float4> tex : register(t0);

float4 main(v2f IN) : SV_Target0
{
	return tex.Load(int3(IN.uv.xy*1024.0f, 0));
}

)EOSHADER";

struct a2v
{
  Vec3f pos;
  Vec4f col;
  Vec2f uv;
};

string common = R"EOSHADER(

#version 420 core

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
};

)EOSHADER";

string vertex = R"EOSHADER(

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 UV;

layout(location = 0) out v2f vertOut;

uniform vec2 wave;

void main()
{
	vertOut.pos = vec4(Position.xyz, 1);
  vertOut.pos.xy += wave*0.2f;
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xy, 0, 1);
}

)EOSHADER";

string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(binding = 0) uniform sampler2D tex2D;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = textureLod(tex2D, vertIn.uv.xy, 0.0f);
}

)EOSHADER";

struct d3dimpl : D3D11GraphicsTest
{
  int main(int argc, char **argv) { return 1; }
};

struct impl : OpenGLGraphicsTest
{
  int main(int argc, char **argv);

  d3dimpl d3d;

  ID3D11VertexShaderPtr vs;
  ID3D11PixelShaderPtr ps;

  ID3D11Texture2DPtr d3d_fromd3d;
  ID3D11Texture2DPtr d3d_tod3d;
  ID3D11ShaderResourceViewPtr srv;
  ID3D11RenderTargetViewPtr rtv;
  ID3D11RenderTargetViewPtr rtv2;

  ID3D11BufferPtr buf;

  HANDLE interop_dev;
  HANDLE interop_fromd3d;
  GLuint gl_fromd3d;
  HANDLE interop_tod3d;
  GLuint gl_tod3d;

  HANDLE interop_d3dbuf;

  GLuint vao;
  GLuint vb;

  GLuint fbo;

  GLuint program;
};

int impl::main(int argc, char **argv)
{
  d3d.headless = true;

  if(!d3d.Init(argc, argv))
    return 4;

  HRESULT hr = S_OK;

  ID3DBlobPtr vsblob = d3d.Compile(dxcommon + dxvertex, "main", "vs_5_0");
  ID3DBlobPtr psblob = d3d.Compile(dxcommon + dxpixel, "main", "ps_5_0");

  CHECK_HR(
      d3d.dev->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, &vs));
  CHECK_HR(d3d.dev->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, &ps));

  d3d.MakeTexture2D(1024, 1024, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &d3d_fromd3d, NULL, NULL, &rtv, NULL);

  d3d.MakeTexture2D(1024, 1024, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &d3d_tod3d, &srv, NULL, &rtv2, NULL);

  float black[4] = {};
  d3d.ctx->ClearRenderTargetView(rtv2, black);

  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  HDC dc = GetDC(glfwGetWin32Window(win));
  gladLoadWGL(dc);
  ReleaseDC(NULL, dc);

  a2v triangle[] = {
      {
          Vec3f(-0.8f, -0.8f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(-0.8f, 0.8f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.8f, -0.8f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.8f, 0.8f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 1.0f),
      },
  };

  d3d.MakeBuffer(D3D11GraphicsTest::eCBuffer, 0, sizeof(triangle), 0, DXGI_FORMAT_UNKNOWN, triangle,
                 &buf, NULL, NULL, NULL);

  vao = MakeVAO();
  glBindVertexArray(vao);

  vb = MakeBuffer();
  glBindBuffer(GL_ARRAY_BUFFER, vb);

  interop_dev = wglDXOpenDeviceNV(d3d.dev);

  interop_d3dbuf = wglDXRegisterObjectNV(interop_dev, buf, vb, GL_NONE, WGL_ACCESS_READ_ONLY_NV);

  // glBufferStorage(GL_ARRAY_BUFFER, sizeof(triangle), triangle, 0);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(0));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(sizeof(Vec3f)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(a2v),
                        (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  program = MakeProgram(common + vertex, common + pixel);
  glObjectLabel(GL_PROGRAM, program, -1, "Full program");

  gl_fromd3d = MakeTexture();
  interop_fromd3d = wglDXRegisterObjectNV(interop_dev, d3d_fromd3d, gl_fromd3d, GL_TEXTURE_2D,
                                          WGL_ACCESS_READ_ONLY_NV);

  gl_tod3d = MakeTexture();
  interop_tod3d = wglDXRegisterObjectNV(interop_dev, d3d_tod3d, gl_tod3d, GL_TEXTURE_2D,
                                        WGL_ACCESS_READ_WRITE_NV);

  fbo = MakeFBO();
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_tod3d, 0);

  GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, bufs);

  glDepthFunc(GL_ALWAYS);
  glDisable(GL_DEPTH_TEST);

  GLenum fbostatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  ID3D11DeviceContextPtr ctx = d3d.ctx;

  HANDLE lockHandles[] = {interop_tod3d, interop_fromd3d};

  float delta = 0.0f;

  bool capd = false;

  int frame = 0;

  while(Running())
  {
    frame++;

    wglDXLockObjectsNV(interop_dev, 1, &interop_d3dbuf);

    float col2[] = {0.6f, 0.4f, 0.6f, 1.0f};
    ctx->ClearRenderTargetView(rtv, col2);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(vs, NULL, 0);
    ctx->PSSetShader(ps, NULL, 0);

    D3D11_VIEWPORT view = {0.0f, 0.0f, 1024.0f, 1024.0f, 0.0f, 1.0f};
    ctx->RSSetViewports(1, &view);

    ctx->OMSetRenderTargets(1, &rtv.GetInterfacePtr(), NULL);

    ctx->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());

    ctx->Draw(4, 0);

    ctx->ClearState();

    wglDXLockObjectsNV(interop_dev, ARRAY_COUNT(lockHandles), lockHandles);

    glBindVertexArray(vao);

    glUseProgram(program);

    glUniform2f(glGetUniformLocation(program, "wave"), sinf(delta * 0.9f), -cosf(delta * 2.7f));

    delta += 0.1f;

    glBindTexture(GL_TEXTURE_2D, gl_fromd3d);

    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};

    // render back into d3d
    glViewport(0, 0, 1024, 1024);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClearBufferfv(GL_COLOR, 0, col);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // render to the backbuffer for visualisation
    glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearBufferfv(GL_COLOR, 0, col);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    wglDXUnlockObjectsNV(interop_dev, ARRAY_COUNT(lockHandles), lockHandles);

    wglDXUnlockObjectsNV(interop_dev, 1, &interop_d3dbuf);

    Present();
  }

  wglDXUnregisterObjectNV(interop_dev, interop_d3dbuf);
  wglDXUnregisterObjectNV(interop_dev, interop_fromd3d);
  wglDXUnregisterObjectNV(interop_dev, interop_tod3d);
  wglDXCloseDeviceNV(interop_dev);

  return 0;
}

};    // anonymous namespace

REGISTER_TEST("GL", "DX_Interop",
              "Test interop between GL and DX (Create and render to a DX surface and include into "
              "GL rendering)");