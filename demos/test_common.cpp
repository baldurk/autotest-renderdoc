/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2016 Baldur Karlsson
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

#include "test_common.h"
#include <stdarg.h>

const DefaultA2V DefaultTri[3] = {
    {Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f)},
    {Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f)},
    {Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f)},
};

static char printBuf[4096] = {};

void DebugPrint(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  vsnprintf(printBuf, 4095, fmt, args);

  va_end(args);

  fputs(printBuf, stdout);
  fflush(stdout);

#if defined(WIN32)
  OutputDebugStringA(printBuf);
#endif
}

bool SpvCompilationSupported()
{
  FILE *pipe = popen("glslc --help", "r");

  if(!pipe)
    return false;

  msleep(20);

  int code = pclose(pipe);

  return WEXITSTATUS(code) == 0;
}

std::vector<uint32_t> CompileShaderToSpv(const std::string &source_text, ShaderLang lang,
                                         ShaderStage stage, const char *entry_point)
{
  std::vector<uint32_t> ret;

  std::string command_line = "glslc -g -O0";

  command_line += " -fentry-point=";
  command_line += entry_point;

  if(lang == ShaderLang::glsl)
    command_line += " -x glsl";
  else if(lang == ShaderLang::hlsl)
    command_line += " -x hlsl";

  switch(stage)
  {
    case ShaderStage::vert: command_line += " -fshader-stage=vert"; break;
    case ShaderStage::frag: command_line += " -fshader-stage=frag"; break;
    case ShaderStage::tesscontrol: command_line += " -fshader-stage=tesscontrol"; break;
    case ShaderStage::tesseval: command_line += " -fshader-stage=tesseval"; break;
    case ShaderStage::geom: command_line += " -fshader-stage=geom"; break;
    case ShaderStage::comp: command_line += " -fshader-stage=comp"; break;
  }

  char infile[MAX_PATH] = {};
  char outfile[MAX_PATH] = {};
  tmpnam(infile);
  tmpnam(outfile);

  command_line += " -o ";
  command_line += outfile;
  command_line += " ";
  command_line += infile;

  FILE *f = fopen(infile, "wb");
  if(f)
  {
    fwrite(source_text.c_str(), 1, source_text.size(), f);
    fclose(f);
  }

  FILE *pipe = popen(command_line.c_str(), "r");

  if(!pipe)
  {
    TEST_ERROR("Couldn't run shaderc to compile shaders.");
    return ret;
  }

  msleep(100);

  int code = pclose(pipe);

  if(code != 0)
  {
    TEST_ERROR("Invoking shaderc failed: %s.", command_line.c_str());
    return ret;
  }

  f = fopen(outfile, "rb");
  if(f)
  {
    fseek(f, 0, SEEK_END);
    ret.resize(ftell(f) / sizeof(uint32_t));
    fseek(f, 0, SEEK_SET);
    fread(&ret[0], sizeof(uint32_t), ret.size(), f);
    fclose(f);
  }

  unlink(infile);
  unlink(outfile);

  return ret;
}

bool GraphicsTest::Init(int argc, char **argv)
{
  srand(0U);

  // parse parameters
  for(int i = 0; i < argc; i++)
  {
    if(!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-debug") ||
       !strcmp(argv[i], "--validate") || !strcmp(argv[i], "-validate"))
    {
      debugDevice = true;
    }

    if(i + 1 < argc && (!strcmp(argv[i], "--frames") || !strcmp(argv[i], "--framecount") ||
                        !strcmp(argv[i], "--max-frames")))
    {
      maxFrameCount = atoi(argv[i + 1]);
    }
  }

#if defined(WIN32)

  HMODULE mod = GetModuleHandleA("renderdoc.dll");
  if(mod)
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void **)&rdoc);

    if(ret != 1)
      rdoc = NULL;
  }

#else

#endif

  return true;
}

bool GraphicsTest::FrameLimit()
{
  curFrame++;
  if(maxFrameCount > 0 && curFrame >= maxFrameCount)
    return false;

  return true;
}
