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
#include <windows.h>

std::string GetCWD()
{
  char cwd[MAX_PATH + 1] = {0};
  GetCurrentDirectoryA(MAX_PATH, cwd);

  string cwdstr = cwd;

  for(size_t i = 0; i < cwdstr.size(); i++)
    if(cwdstr[i] == '\\')
      cwdstr[i] = '/';

  while(cwdstr.back() == '/' || cwdstr.back() == '\\')
    cwdstr.pop_back();

  return cwdstr;
}

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
  FILE *pipe = _popen("glslc --help", "r");

  if(!pipe)
    return false;

  int code = _pclose(pipe);

  return code == 0;
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
  tmpnam_s(infile);
  tmpnam_s(outfile);

  command_line += " -o ";
  command_line += outfile;
  command_line += " ";
  command_line += infile;

  FILE *f = NULL;
  fopen_s(&f, infile, "wb");
  if(f)
  {
    fwrite(source_text.c_str(), 1, source_text.size(), f);
    fclose(f);
  }

  FILE *pipe = _popen(command_line.c_str(), "r");

  if(!pipe)
  {
    TEST_ERROR("Couldn't run shaderc to compile shaders.");
    return ret;
  }

  int code = _pclose(pipe);

  if(code != 0)
  {
    TEST_ERROR("Invoking shaderc failed: %s.", command_line.c_str());
    return ret;
  }

  f = NULL;
  fopen_s(&f, outfile, "rb");
  if(f)
  {
    fseek(f, 0, SEEK_END);
    ret.resize(ftell(f) / sizeof(uint32_t));
    fseek(f, 0, SEEK_SET);
    fread(&ret[0], sizeof(uint32_t), ret.size(), f);
    fclose(f);
  }

  _unlink(infile);
  _unlink(outfile);

  return ret;
}
