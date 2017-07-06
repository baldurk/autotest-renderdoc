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

#include "../gl_common.h"

namespace
{
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

void main()
{
	vertOut.pos = vec4(Position.xyz, 1);
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xy, 0, 1);
}

)EOSHADER";

string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col;
}

)EOSHADER";

struct impl : OpenGLGraphicsTest
{
  int main(int argc, char **argv);

  GLuint vao;
  GLuint vb;

  GLuint fbo;
  GLuint attachments[3];
  static const GLsizei numSamples = 8;

  GLuint program;
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  a2v triangle[] = {
      {
          Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  vao = MakeVAO();
  glBindVertexArray(vao);

  vb = MakeBuffer();
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glBufferStorage(GL_ARRAY_BUFFER, sizeof(triangle), triangle, 0);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(0));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(sizeof(Vec3f)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(a2v),
                        (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  program = MakeProgram(common + vertex, common + pixel);

  fbo = MakeFBO();
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Color render texture
  for(int i = 0; i < 3; i++)
    attachments[i] = MakeTexture();

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachments[0]);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGB10_A2, screenWidth,
                          screenHeight, false);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                         attachments[0], 0);

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachments[1]);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGB, screenWidth, screenHeight,
                          false);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                         attachments[1], 0);

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachments[2]);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_DEPTH_COMPONENT24, screenWidth,
                          screenHeight, false);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                         attachments[2], 0);

  glDepthFunc(GL_ALWAYS);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  while(Running())
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);

    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, col);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    glBindVertexArray(vao);

    glUseProgram(program);

    glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK_LEFT);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int GL_Unsized_MS_FBO_Attachment(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}
