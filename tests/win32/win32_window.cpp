/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2018 Baldur Karlsson
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

#include "win32_window.h"
#include <windows.h>

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

const char *classname = "renderdoc_d3d11_test";

void regClass()
{
  static bool init = false;
  if(init)
    return;

  init = true;

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
  wc.lpszClassName = classname;
  wc.hIconSm = NULL;

  if(!RegisterClassEx(&wc))
  {
    TEST_ERROR("Couldn't register window class");
    return;
  }
}

Win32Window::Win32Window(int width, int height, const char *title)
{
  regClass();

  RECT rect = {0, 0, width, height};
  AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

  wnd = CreateWindowExA(WS_EX_CLIENTEDGE, classname, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                        CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL,
                        NULL, NULL);
  ShowWindow(wnd, SW_SHOW);
}

Win32Window::~Win32Window()
{
  DestroyWindow(wnd);
}

void Win32Window::Resize(int width, int height)
{
  SetWindowPos(wnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
}

bool Win32Window::Update()
{
  UpdateWindow(wnd);

  MSG msg = {};

  // Check to see if any messages are waiting in the queue
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    // Translate the message and dispatch it to WindowProc()
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if(!IsWindowVisible(wnd))
    return false;

  // If the message is WM_QUIT, exit the program
  if(msg.message == WM_QUIT)
    return false;

  if(msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
    return false;

  Sleep(20);

  return true;
}
