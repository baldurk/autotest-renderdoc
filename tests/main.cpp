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

#include "test_common.h"

bool GraphicsTest::Init(int argc, char **argv)
{
	// parse parameters

	return true;
}

#define STRINGIZE(a) #a

#define TEST(testname) int testname(int,char**); if(!strcmp(argv[1], STRINGIZE(testname))) { return testname(argc, argv); }

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		TEST_ERROR("Invalid usage: %s <test-name>", argv[0]);
		return 1;
	}

	//////////////////////////////////////////////////////////////
	// D3D11 tests
	//////////////////////////////////////////////////////////////

	// Just draws a simple triangle, using normal pipeline. Basic test
	// that can be used for any dead-simple tests that don't require any
	// particular API use
	TEST(D3D11_Simple_Triangle);

	// Renders a lot of overlapping triangles
	TEST(D3D11_Overdraw_Stress);

	// Tests simple shader debugging identities by rendering many small
	// triangles and performing one calculation to each to an F32 target
	TEST(D3D11_Debug_Shader);

	// Test using more than 8 compute shader UAVs (D3D11.1 feature)
	TEST(D3D11_1_Many_UAVs);

	// Test rendering into RTV mip levels
	TEST(D3D11_Mip_RTV);

	// Test repeatedly creating and destroying RTVs
	TEST(D3D11_Many_RTVs);

	// Test dispatching with one threadgroup count set to 0
	TEST(D3D11_Empty_Compute_Dispatch);

	// Test passing an array of float2 to make sure the interpolator
	// packing is handled by shader debugging
	TEST(D3D11_Array_Interpolator);

	// Test a drawcall of 0 size
	TEST(D3D11_Empty_Drawcall);

	//////////////////////////////////////////////////////////////
	// OpenGL tests
	//////////////////////////////////////////////////////////////

	// Just draws a simple triangle, using normal pipeline. Basic test
	// that can be used for any dead-simple tests that don't require any
	// particular API use
	TEST(GL_Simple_Triangle);
	
	TEST_ERROR("%s is not a known test", argv[1]);

	return 2;
}