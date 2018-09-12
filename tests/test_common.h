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

#if defined(WIN32)
#include "win32_platform.h"
#else
#include "linux_platform.h"
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>

typedef uint8_t byte;

enum class ShaderLang
{
  glsl,
  hlsl
};
enum class ShaderStage
{
  vert,
  frag,
  tesscontrol,
  tesseval,
  geom,
  comp
};

bool SpvCompilationSupported();
std::vector<uint32_t> CompileShaderToSpv(const std::string &source_text, ShaderLang lang,
                                         ShaderStage stage, const char *entry_point);

struct Vec2f
{
  Vec2f(float X = 0.0f, float Y = 0.0f)
  {
    x = X;
    y = Y;
  }
  float x;
  float y;
};

class Vec3f
{
public:
  Vec3f(const float X = 0.0f, const float Y = 0.0f, const float Z = 0.0f) : x(X), y(Y), z(Z) {}
  inline float Dot(const Vec3f &o) const { return x * o.x + y * o.y + z * o.z; }
  inline Vec3f Cross(const Vec3f &o) const
  {
    return Vec3f(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
  }

  inline float Length() const { return sqrt(Dot(*this)); }
  inline void Normalise()
  {
    float l = Length();
    x /= l;
    y /= l;
    z /= l;
  }

  float x, y, z;
};

struct Vec4f
{
  Vec4f(float X = 0.0f, float Y = 0.0f, float Z = 0.0f, float W = 0.0f)
  {
    x = X;
    y = Y;
    z = Z;
    w = W;
  }
  float x, y, z, w;
};

struct DefaultA2V
{
  Vec3f pos;
  Vec4f col;
  Vec2f uv;
};

extern const DefaultA2V DefaultTri[3];

struct Window
{
  virtual ~Window() {}
  virtual void Resize(int width, int height) = 0;
  virtual bool Update() = 0;
};

struct GraphicsTest
{
  GraphicsTest()
      : screenWidth(1280), screenHeight(720), fullscreen(false), debugDevice(false), headless(false)
  {
  }

  virtual ~GraphicsTest() {}
  virtual Window *MakeWindow(int width, int height, const char *title) { return NULL; }
  virtual int main(int argc, char **argv) { return 9; }
  virtual bool Init(int argc, char **argv);

  void StartFrameCapture(void *device = NULL, void *wnd = NULL);
  void EndFrameCapture(void *device = NULL, void *wnd = NULL);

  int screenWidth;
  int screenHeight;
  bool fullscreen;
  bool debugDevice;
  bool headless;
};

enum class TestAPI
{
  D3D11,
  Vulkan,
  OpenGL,
};

struct TestMetadata
{
  TestAPI API;
  const char *Name;
  const char *Description;
  GraphicsTest *test;

  const char *APIName() const
  {
    switch(API)
    {
      case TestAPI::D3D11: return "D3D11";
      case TestAPI::Vulkan: return "VK";
      case TestAPI::OpenGL: return "GL";
    }

    return "???";
  }

  bool operator<(const TestMetadata &o)
  {
    if(API != o.API)
      return API < o.API;

    int ret = strcmp(Name, o.Name);
    if(ret != 0)
      return ret < 0;

    return test < o.test;
  }
};

void RegisterTest(TestMetadata test);

#define REGISTER_TEST(TestName)                 \
  namespace                                     \
  {                                             \
  struct TestRegistration                       \
  {                                             \
    TestName m_impl;                            \
    TestRegistration()                          \
    {                                           \
      TestMetadata test;                        \
      test.API = TestName::API;                 \
      test.Name = #TestName;                    \
      test.Description = TestName::Description; \
      test.test = &m_impl;                      \
      RegisterTest(test);                       \
    }                                           \
  };                                            \
  };                                            \
  static TestRegistration Anon##__LINE__;

std::string GetCWD();

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define RANDF(mn, mx) ((float(rand()) / float(RAND_MAX)) * ((mx) - (mn)) + (mn))

void DebugPrint(const char *fmt, ...);

#define TEST_ASSERT(cond, fmt, ...)                                                               \
  if(!(cond))                                                                                     \
  {                                                                                               \
    DebugPrint("%s:%d Assert Failure '%s': " fmt "\n", __FILE__, __LINE__, #cond, ##__VA_ARGS__); \
    DEBUG_BREAK();                                                                                \
  }

#define TEST_LOG(fmt, ...)                                                 \
  do                                                                       \
  {                                                                        \
    DebugPrint("%s:%d Log: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#define TEST_WARN(fmt, ...)                                                    \
  do                                                                           \
  {                                                                            \
    DebugPrint("%s:%d Warning: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while(0)
#define TEST_ERROR(fmt, ...)                                                 \
  do                                                                         \
  {                                                                          \
    DebugPrint("%s:%d Error: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    DEBUG_BREAK();                                                           \
  } while(0)
#define TEST_FATAL(fmt, ...)                                                       \
  do                                                                               \
  {                                                                                \
    DebugPrint("%s:%d Fatal Error: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    DEBUG_BREAK();                                                                 \
    exit(0);                                                                       \
  } while(0)
#define TEST_UNIMPLEMENTED(fmt, ...)                                                 \
  do                                                                                 \
  {                                                                                  \
    DebugPrint("%s:%d Unimplemented: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    DEBUG_BREAK();                                                                   \
    exit(0);                                                                         \
  } while(0)
