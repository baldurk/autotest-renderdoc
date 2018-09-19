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

#include "test_common.h"

#pragma warning(push)
#pragma warning(disable : 4127)    // conditional expression is constant
#pragma warning(disable : 4244)    // conversion from 'x' to 'y', possible loss of data
#pragma warning(disable : 4505)    // unreferenced local function has been removed
#pragma warning(disable : 4701)    // potentially uninitialized local variable used

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_ASSERT(expr) TEST_ASSERT(expr, "nuklear assertion failed")
#include "nuklear/nuklear.h"

#if defined(WIN32)

#define NK_GDI_IMPLEMENTATION
#include "nuklear/nuklear_gdi.h"

static LRESULT CALLBACK NuklearWndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if(msg == WM_CLOSE)
  {
    DestroyWindow(wnd);
    return 0;
  }
  if(msg == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  }

  if(nk_gdi_handle_event(wnd, msg, wparam, lparam))
    return 0;

  return DefWindowProcW(wnd, msg, wparam, lparam);
}

WNDCLASSW wc = {};
HWND wnd = NULL;
HDC dc = NULL;
GdiFont *font = NULL;

nk_context *NuklearInit(int width, int height, const char *title)
{
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = NuklearWndProc;
  wc.hInstance = GetModuleHandleA(NULL);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = L"NuklearWindowClass";
  RegisterClassW(&wc);

  int len = (int)strlen(title);

  int wsize = MultiByteToWideChar(CP_UTF8, 0, title, len, NULL, 0);
  WCHAR *wstr = (WCHAR *)_alloca(wsize * sizeof(wchar_t) + 2);
  wstr[wsize] = 0;
  MultiByteToWideChar(CP_UTF8, 0, title, len, wstr, wsize);

  RECT rect = {0, 0, width, height};
  AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_SYSMENU, FALSE, WS_EX_WINDOWEDGE);
  wnd = CreateWindowExW(
      WS_EX_WINDOWEDGE, wc.lpszClassName, wstr, WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE,
      (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2,
      (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2, rect.right - rect.left,
      rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);

  dc = GetDC(wnd);

  font = nk_gdifont_create("Arial", 14);
  return nk_gdi_init(font, dc, width, height);
}

bool NuklearTick(nk_context *ctx)
{
  MSG msg = {};
  UpdateWindow(wnd);

  nk_input_begin(ctx);
  // Check to see if any messages are waiting in the queue
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    // Translate the message and dispatch it to WindowProc()
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  nk_input_end(ctx);

  if(!IsWindowVisible(wnd) || msg.message == WM_QUIT)
    return false;

  return true;
}

void NuklearShutdown()
{
  nk_gdifont_del(font);
  ReleaseDC(wnd, dc);
  DestroyWindow(wnd);
  UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

#else

#define NK_XLIB_IMPLEMENTATION
#include "nuklear/nuklear_xlib.h"

#endif

// nuklear
#pragma warning(pop)

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

  std::sort(tests.begin(), tests.end());

  if(argc >= 2 && !strcmp(argv[1], "--list"))
  {
    for(const TestMetadata &test : tests)
      printf("%s (%s) - %s\n", test.Name, test.APIName(), test.Description);

    fflush(stdout);
    return 1;
  }

  std::string testchoice;

  if(argc >= 2)
  {
    testchoice = argv[1];
  }
  else
  {
    nk_context *ctx = NuklearInit(400, 300, "RenderDoc Test Program");
    size_t curtest = 0;
    bool allow[(int)TestAPI::Count] = {};
    const char *allow_names[] = {
        "D3D11", "Vulkan", "OpenGL",
    };

    for(size_t i = 0; i < ARRAY_COUNT(allow); i++)
      allow[i] = true;

    static_assert(ARRAY_COUNT(allow) == ARRAY_COUNT(allow_names), "Mismatched array");

    while(NuklearTick(ctx))
    {
      if(nk_begin(ctx, "Demo", nk_rect(0, 0, 400, 300), 0))
      {
        nk_layout_row_dynamic(ctx, 30, 4);

        nk_label(ctx, "Filter tests:", NK_TEXT_LEFT);

        for(size_t i = 0; i < ARRAY_COUNT(allow); i++)
        {
          std::string text = allow_names[i];
          if(allow[i])
            text = "Include " + text;
          else
            text = "Exclude " + text;

          bool newstate = nk_check_label(ctx, text.c_str(), allow[i]) != 0;

          // disallow disabling last filter
          bool otherEnabled = false;
          for(size_t j = 0; j < ARRAY_COUNT(allow); j++)
          {
            if(i == j)
              continue;

            if(allow[j])
            {
              otherEnabled = true;
              break;
            }
          }

          if(otherEnabled)
            allow[i] = newstate;
        }

        if(!allow[(int)tests[curtest].API])
        {
          // if the current test is no longer allowed, select the first one that is allowed
          for(size_t i = 0; i < tests.size(); i++)
          {
            if(!allow[(int)tests[i].API])
              continue;

            curtest = i;
            break;
          }
        }

        nk_layout_row_dynamic(ctx, 30, 2);

        nk_label(ctx, "Test:", NK_TEXT_LEFT);

        if(nk_combo_begin_label(ctx, tests[curtest].QualifiedName().c_str(), nk_vec2(400, 200)))
        {
          nk_layout_row_dynamic(ctx, 30, 1);
          for(size_t i = 0; i < tests.size(); i++)
          {
            if(!allow[(int)tests[i].API])
              continue;

            if(nk_combo_item_label(ctx, tests[i].QualifiedName().c_str(), NK_TEXT_LEFT))
              curtest = i;
          }
          nk_combo_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 100, 1);

        TestMetadata &selected_test = tests[curtest];

        if(nk_group_begin(ctx, "Test Information", 0))
        {
          nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 2);
          nk_layout_row_push(ctx, 0.25f);
          nk_label(ctx, "Test name:", NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_RIGHT);
          nk_layout_row_push(ctx, 0.75f);
          nk_label(ctx, selected_test.Name, NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_LEFT);
          nk_layout_row_end(ctx);

          nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 2);
          nk_layout_row_push(ctx, 0.25f);
          nk_label(ctx, "API:", NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_RIGHT);
          nk_layout_row_push(ctx, 0.75f);
          nk_label(ctx, selected_test.APIName(), NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_LEFT);
          nk_layout_row_end(ctx);

          nk_layout_row_begin(ctx, NK_DYNAMIC, 0, 2);
          nk_layout_row_push(ctx, 0.25f);
          nk_label(ctx, "Description:", NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_RIGHT);
          nk_layout_row_push(ctx, 0.75f);
          nk_label_wrap(ctx, selected_test.Description);
          nk_layout_row_end(ctx);

          nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 30, 1);

        if(nk_button_label(ctx, "Run"))
        {
          testchoice = selected_test.Name;
          break;
        }
      }
      nk_end(ctx);

      nk_gdi_render(nk_rgb(30, 30, 30));
    }

    NuklearShutdown();
  }

  if(testchoice.empty())
    return 0;

#if defined(WIN32)
  if(AllocConsole())
  {
    FILE *dummy = NULL;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
  }
#endif

  //////////////////////////////////////////////////////////////
  // D3D11 tests
  //////////////////////////////////////////////////////////////

  for(const TestMetadata &test : tests)
  {
    if(testchoice == test.Name)
    {
      TEST_LOG("\n\n======\nRunning %s\n\n", test.Name);
      int ret = test.test->main(argc, argv);
      return ret;
    }
  }

  TEST_ERROR("%s is not a known test", argv[1]);

  return 2;
}

#if defined(WIN32)
int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                    _In_ int nShowCmd)
{
  LPWSTR *wargv;
  int argc;

  if(AttachConsole(ATTACH_PARENT_PROCESS))
  {
    FILE *dummy = NULL;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
  }

  wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

  char **argv = new char *[argc];
  for(int i = 0; i < argc; i++)
  {
    // allocate pessimistically
    int allocSize = (int)wcslen(wargv[i]) * 4 + 1;

    argv[i] = new char[allocSize];

    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &argv[i][0], allocSize, NULL, NULL);
  }

  main(argc, argv);

  delete[] argv;
  LocalFree(wargv);
}

#endif
