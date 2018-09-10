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

  GLuint vao, vao2;
  GLuint vb;

  GLuint program;

  GLFWwindow *win2;
};

int impl::main(int argc, char **argv)
{
  debugDevice = true;

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
  glObjectLabel(GL_PROGRAM, program, -1, "Full program");

  glfwWindowHint(GLFW_DEPTH_BITS, 0);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);

  win2 = glfwCreateWindow(400, 300, "Autotesting 2", NULL, win);

  glfwMakeContextCurrent(win2);

  glGenVertexArrays(1, &vao2);
  glBindVertexArray(vao2);

  glBindBuffer(GL_ARRAY_BUFFER, vb);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(0));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(sizeof(Vec3f)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(a2v),
                        (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  while(Running())
  {
    glfwMakeContextCurrent(win);

    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, col);

    glBindVertexArray(vao);

    glUseProgram(program);

    glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    Present();

    glfwMakeContextCurrent(win2);

    float col2[] = {0.6f, 0.5f, 0.4f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, col2);

    glBindVertexArray(vao2);

    glUseProgram(program);

    glViewport(0, 0, GLsizei(400), GLsizei(300));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(win2);
    glfwPollEvents();
  }

  glfwDestroyWindow(win2);
  glDeleteVertexArrays(1, &vao2);

  return 0;
}

};    // anonymous namespace

REGISTER_TEST(
    "GL", "Multi_Window",
    "Render to two different windows to test out different contexts and window resolutions.");