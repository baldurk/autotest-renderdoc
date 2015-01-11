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

namespace {

string common = R"EOSHADER(
)EOSHADER";

string vertex = R"EOSHADER(
)EOSHADER";

string pixel = R"EOSHADER(
)EOSHADER";

struct impl : OpenGLGraphicsTest
{
	int main(int argc, char **argv);
};

int impl::main(int argc, char **argv)
{
	// initialise, create window, create context, etc
	if(!Init(argc, argv))
		return 3;

	// make shaders, resources, etc here.

	while(Running())
	{
		Present();
	}

	return 0;
}

}; // anonymous namespace

int GL_Simple_Triangle(int argc, char **argv) { impl i; return i.main(argc, argv); }