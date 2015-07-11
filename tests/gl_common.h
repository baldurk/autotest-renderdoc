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

#include "test_common.h"

#include <windows.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <vector>

struct OpenGLGraphicsTest : public GraphicsTest
{
	OpenGLGraphicsTest()
		: glMajor(4)
		, glMinor(3)
	{
#ifdef WIN32
		wnd = NULL;
		dc = NULL;
		rc = NULL;
#endif
	}

	~OpenGLGraphicsTest();

	bool Init(int argc, char **argv);
	
	GLuint MakeProgram(string vertSrc, string fragSrc);
	GLuint MakeBuffer();
	GLuint MakeTexture();
	GLuint MakeVAO();

	bool Running();
	void Present();

	int glMajor;
	int glMinor;

#ifdef WIN32
	HWND wnd;
	HDC dc;
	HGLRC rc;
#endif

	std::vector<GLuint> bufs, texs, progs, vaos;
};