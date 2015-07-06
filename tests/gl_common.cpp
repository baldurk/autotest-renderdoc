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

static void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	// too much spam on these types
	if(type != GL_DEBUG_TYPE_PERFORMANCE && type != GL_DEBUG_TYPE_OTHER)
	{
		TEST_ERROR("Debug message: %s", message);
	}
}

#ifdef WIN32
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_CLOSE)   { DestroyWindow(hwnd); return 0; }
	if(msg == WM_DESTROY) { PostQuitMessage(0);  return 0; }
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

bool OpenGLGraphicsTest::Init(int argc, char **argv)
{
	// parse parameters here to override parameters
	GraphicsTest::Init(argc, argv);

	// GL specific can go here
	// ...
	
#ifdef WIN32
	WNDCLASSEXA wc;
	wc.cbSize        = sizeof(WNDCLASSEXA);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "renderdoc_test";
	wc.hIconSm       = NULL;

	if(!RegisterClassEx(&wc))
	{
		return 1;
	}
	
	wnd = CreateWindowExA(WS_EX_CLIENTEDGE, "renderdoc_test", "RenderDoc test program", WS_OVERLAPPEDWINDOW,
	                          CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight, NULL, NULL, NULL, NULL);
	
	PIXELFORMATDESCRIPTOR pfd = { 0 };

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0;
	pfd.cStencilBits = 0;
	
	dc = GetDC(wnd);
	
	int pf = ChoosePixelFormat(dc, &pfd);
	if(pf == 0)
	{
		TEST_ERROR("Couldn't choose pixel format");
		return 1;
	}

	BOOL res = SetPixelFormat(dc, pf, &pfd);
	if(res == FALSE)
	{
		TEST_ERROR("Couldn't set pixel format");
		return 1;
	}
	
	HGLRC glrc = wglCreateContext(dc);
	if(glrc == NULL)
	{
		TEST_ERROR("Couldn't create simple RC");
		return 1;
	}

	res = wglMakeCurrent(dc, glrc);
	if(res == FALSE)
	{
		TEST_ERROR("Couldn't make simple RC current");
		return 1;
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC createContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC getPixelFormatAttrib = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");

	if(createContextAttribs == NULL || getPixelFormatAttrib == NULL)
	{
		TEST_ERROR("can't find WGL_ARB_create_context or WGL_ARB_pixel_format");
		return 1;
	}
	
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(glrc);
	ReleaseDC(wnd, dc);
	dc = GetDC(wnd);
	
	pf = ChoosePixelFormat(dc, &pfd);
	if(pf == 0)
	{
		TEST_ERROR("Couldn't choose pixel format");
		return 1;
	}

	res = SetPixelFormat(dc, pf, &pfd);
	if(res == FALSE)
	{
		TEST_ERROR("Couldn't set pixel format");
		return 1;
	}

	int attribs[64] = {0};
	int i=0;

	attribs[i++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
	attribs[i++] = glMajor;
	attribs[i++] = WGL_CONTEXT_MINOR_VERSION_ARB;
	attribs[i++] = glMinor;
	attribs[i++] = WGL_CONTEXT_FLAGS_ARB;
	attribs[i++] = debugDevice ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;
	attribs[i++] = WGL_CONTEXT_PROFILE_MASK_ARB;
	attribs[i++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

	rc = createContextAttribs(dc, NULL, attribs);
	if(rc == NULL)
	{
		TEST_ERROR("Couldn't create 4.3 RC - RenderDoc requires OpenGL 4.3 availability");
		return 1;
	}

	res = wglMakeCurrent(dc, rc);
	if(res == FALSE)
	{
		TEST_ERROR("Couldn't make 4.3 RC current");
		return 1;
	}

	// this is required to get glew to work on core profiles, as it unconditionally calls
	// glGetString(GL_EXTENSIONS) even though that's invalid on core profiles
	glewExperimental = true;

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		TEST_ERROR("Error initialising glew. Error: %s", glewGetErrorString(err));
	}
	
	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);
#endif

	glDebugMessageCallback(&debugCallback, NULL);

	return true;
}

OpenGLGraphicsTest::~OpenGLGraphicsTest()
{
	if (rc) wglDeleteContext(rc);
	if (dc) ReleaseDC(wnd, dc);
	if (wnd) DestroyWindow(wnd);
}

GLuint OpenGLGraphicsTest::MakeProgram(string vertSrc, string fragSrc)
{
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	const char *cstr = vertSrc.c_str();
	glShaderSource(vs, 1, &cstr, NULL);
	cstr = fragSrc.c_str();
	glShaderSource(fs, 1, &cstr, NULL);

	glCompileShader(vs);
	glCompileShader(fs);
	
	char buffer[1024];
	GLint status = 0;

	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if(status == 0)
	{
		glGetShaderInfoLog(vs, 1024, NULL, buffer);
		TEST_ERROR("Shader error: %s", buffer);
		glDeleteShader(vs);
		glDeleteShader(fs);
		return 0;
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if(status == 0)
	{
		glGetShaderInfoLog(fs, 1024, NULL, buffer);
		TEST_ERROR("Shader error: %s", buffer);
		glDeleteShader(vs);
		glDeleteShader(fs);
		return 0;
	}
	
	GLuint program = glCreateProgram();

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == 0)
	{
		glGetProgramInfoLog(program, 1024, NULL, buffer);
		TEST_ERROR("Link error: %s", buffer);

		glDeleteProgram(program);
		program = 0;
	}

	glDetachShader(program, vs);
	glDetachShader(program, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

bool OpenGLGraphicsTest::Running()
{
#ifdef WIN32
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	// Check to see if any messages are waiting in the queue
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// Translate the message and dispatch it to WindowProc()
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// If the message is WM_QUIT, exit the program
	if (msg.message == WM_QUIT) return false;

	Sleep(20);
#endif

	return true;
}

void OpenGLGraphicsTest::Present()
{
#ifdef WIN32
	SwapBuffers(dc);
#endif
}
