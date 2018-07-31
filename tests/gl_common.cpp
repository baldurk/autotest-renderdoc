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

#include "gl_common.h"

#include <stdio.h>

static void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                   GLsizei length, const GLchar *message, const void *userParam)
{
  // too much spam on these types
  if(type != GL_DEBUG_TYPE_PERFORMANCE && type != GL_DEBUG_TYPE_OTHER)
  {
    TEST_ERROR("Debug message: %s", message);
  }
}

static void glfwCallback(int err, const char *str)
{
  TEST_ERROR("GLFW Error %i: %s", err, str);
}

static void glfwKeypress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if(key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, 1);
}

bool OpenGLGraphicsTest::Init(int argc, char **argv)
{
  // parse parameters here to override parameters
  GraphicsTest::Init(argc, argv);

  if(!glfwInit())
  {
    TEST_ERROR("Failed to init GLFW");
    return false;
  }

  inited = true;

  glfwSetErrorCallback(&glfwCallback);

  glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
  glfwWindowHint(GLFW_CLIENT_API, gles ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
  glfwWindowHint(GLFW_OPENGL_PROFILE,
                 coreProfile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  glfwWindowHint(GLFW_RESIZABLE, 0);

  win = glfwCreateWindow(screenWidth, screenHeight, "Autotesting", NULL, NULL);

  if(!win)
  {
    TEST_ERROR("Error creating glfw window");
    return false;
  }

  glfwSetKeyCallback(win, &glfwKeypress);

  glfwMakeContextCurrent(win);

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    TEST_ERROR("Error initialising glad");
    return false;
  }

  if(GLAD_GL_ARB_debug_output)
  {
    glDebugMessageCallback(&debugCallback, NULL);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  return true;
}

OpenGLGraphicsTest::~OpenGLGraphicsTest()
{
  if(!bufs.empty())
    glDeleteBuffers((GLsizei)bufs.size(), &bufs[0]);
  if(!texs.empty())
    glDeleteTextures((GLsizei)texs.size(), &texs[0]);
  if(!vaos.empty())
    glDeleteVertexArrays((GLsizei)vaos.size(), &vaos[0]);
  if(!fbos.empty())
    glDeleteFramebuffers((GLsizei)fbos.size(), &fbos[0]);
  if(!pipes.empty())
    glDeleteProgramPipelines((GLsizei)pipes.size(), &pipes[0]);

  for(GLuint p : progs)
    glDeleteProgram(p);

  if(win)
    glfwDestroyWindow(win);
  if(inited)
    glfwTerminate();
}

GLuint OpenGLGraphicsTest::MakeProgram(string vertSrc, string fragSrc, bool sep)
{
  GLuint vs = vertSrc.empty() ? 0 : glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = fragSrc.empty() ? 0 : glCreateShader(GL_FRAGMENT_SHADER);

  const char *cstr = NULL;

  if(vs)
  {
    cstr = vertSrc.c_str();
    glShaderSource(vs, 1, &cstr, NULL);
    glObjectLabel(GL_SHADER, vs, -1, "VS doodad");
    glCompileShader(vs);
  }

  if(fs)
  {
    cstr = fragSrc.c_str();
    glShaderSource(fs, 1, &cstr, NULL);
    glObjectLabel(GL_SHADER, fs, -1, "FS thingy");
    glCompileShader(fs);
  }

  char buffer[1024];
  GLint status = 0;

  if(vs)
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
  else
    status = 1;

  if(status == 0)
  {
    glGetShaderInfoLog(vs, 1024, NULL, buffer);
    TEST_ERROR("Shader error: %s", buffer);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return 0;
  }

  if(fs)
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
  else
    status = 1;

  if(status == 0)
  {
    glGetShaderInfoLog(fs, 1024, NULL, buffer);
    TEST_ERROR("Shader error: %s", buffer);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return 0;
  }

  GLuint program = glCreateProgram();

  if(vs)
    glAttachShader(program, vs);
  if(fs)
    glAttachShader(program, fs);

  if(!vs || !fs || sep)
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if(status == 0)
  {
    glGetProgramInfoLog(program, 1024, NULL, buffer);
    TEST_ERROR("Link error: %s", buffer);

    glDeleteProgram(program);
    program = 0;
  }

  if(vs)
  {
    glDetachShader(program, vs);
    glDeleteShader(vs);
  }
  if(fs)
  {
    glDetachShader(program, fs);
    glDeleteShader(fs);
  }

  if(program)
    progs.push_back(program);

  return program;
}

GLuint OpenGLGraphicsTest::MakeProgram()
{
  GLuint program = glCreateProgram();
  progs.push_back(program);
  return program;
}

GLuint OpenGLGraphicsTest::MakeBuffer()
{
  bufs.push_back(0);
  glGenBuffers(1, &bufs[bufs.size() - 1]);
  return bufs[bufs.size() - 1];
}

GLuint OpenGLGraphicsTest::MakePipeline()
{
  pipes.push_back(0);
  glCreateProgramPipelines(1, &pipes[pipes.size() - 1]);
  return pipes[pipes.size() - 1];
}

GLuint OpenGLGraphicsTest::MakeTexture()
{
  texs.push_back(0);
  glGenTextures(1, &texs[texs.size() - 1]);
  return texs[texs.size() - 1];
}

GLuint OpenGLGraphicsTest::MakeVAO()
{
  vaos.push_back(0);
  glGenVertexArrays(1, &vaos[vaos.size() - 1]);
  return vaos[vaos.size() - 1];
}

GLuint OpenGLGraphicsTest::MakeFBO()
{
  fbos.push_back(0);
  glGenFramebuffers(1, &fbos[fbos.size() - 1]);
  return fbos[fbos.size() - 1];
}

bool OpenGLGraphicsTest::Running()
{
  if(glfwWindowShouldClose(win))
    return false;

  return true;
}

void OpenGLGraphicsTest::Present()
{
  glfwSwapBuffers(win);
  glfwPollEvents();
}
