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

#include <stdio.h>
#include <string.h>
#include <string>

#include "renderdoc_app.h"
#include "test_common.h"

bool GraphicsTest::Init(int argc, char **argv)
{
  // parse parameters
  for(int i = 0; i < argc; i++)
  {
    if(!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-debug") ||
       !strcmp(argv[i], "--validate") || !strcmp(argv[i], "-validate"))
    {
      debugDevice = true;
    }
  }

  return true;
}

void GraphicsTest::StartFrameCapture(void *device, void *wnd)
{
  HMODULE mod = GetModuleHandleA("renderdoc.dll");
  if(mod)
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

    RENDERDOC_API_1_0_0 *rdoc_api = NULL;

    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void **)&rdoc_api);

    if(ret == 1)
      rdoc_api->StartFrameCapture(device, wnd);
  }
}

void GraphicsTest::EndFrameCapture(void *device, void *wnd)
{
  HMODULE mod = GetModuleHandleA("renderdoc.dll");
  if(mod)
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

    RENDERDOC_API_1_0_0 *rdoc_api = NULL;

    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0, (void **)&rdoc_api);

    if(ret == 1)
      rdoc_api->EndFrameCapture(device, wnd);
  }
}

std::vector<TestMetadata> &test_list()
{
  static std::vector<TestMetadata> list;
  return list;
}

void RegisterTest(TestMetadata test)
{
  test_list().push_back(test);
}

int main(int argc, char **argv)
{
  std::vector<TestMetadata> &tests = test_list();

  if(argc >= 2 && !strcmp(argv[1], "--list"))
  {
    for(const TestMetadata &test : tests)
      printf("%s::%s -\n\t%s\n\n", test.APIName(), test.Name, test.Description);

    fflush(stdout);
    return 1;
  }

  std::string testchoice;

  if(argc < 2)
  {
    for(const TestMetadata &test : tests)
      printf("%s::%s -\n\t%s\n\n", test.APIName(), test.Name, test.Description);

    printf("Your choice?\n");

    fflush(stdout);

    char choice[256] = {};
    fgets(choice, 255, stdin);

    testchoice = choice;

    while(isspace(testchoice.back()))
      testchoice.pop_back();
  }
  else
  {
    testchoice = argv[1];
  }

  //////////////////////////////////////////////////////////////
  // D3D11 tests
  //////////////////////////////////////////////////////////////

  for(const TestMetadata &test : tests)
  {
    std::string fullname = test.APIName();
    fullname += "::";
    fullname += test.Name;

    if(testchoice == fullname || testchoice == test.Name)
    {
      TEST_LOG("\n\n======\nRunning %s\n\n", argv[1]);
      int ret = test.test->main(argc, argv);
      return ret;
    }
  }

  TEST_ERROR("%s is not a known test", argv[1]);

  return 2;
}
