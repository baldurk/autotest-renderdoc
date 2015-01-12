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

#include <string>
#include <vector>
using std::string;
using std::vector;

#include <math.h>

struct Vec2f
{
	Vec2f(float X = 0.0f, float Y = 0.0f) { x = X; y = Y; }
	float x;
	float y;
};

class Vec3f
{
public:
	Vec3f(const float X = 0.0f, const float Y = 0.0f, const float Z = 0.0f)
		: x(X), y(Y), z(Z)
	{ }

	inline float Dot(const Vec3f &o) const
	{
		return x*o.x + y*o.y + z*o.z;
	}

	inline Vec3f Cross(const Vec3f &o) const
	{
		return Vec3f(y*o.z - z*o.y,
			z*o.x - x*o.z,
			x*o.y - y*o.x);
	}

	inline float Length() const
	{
		return sqrt(Dot(*this));
	}

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
	Vec4f(float X = 0.0f, float Y = 0.0f, float Z = 0.0f, float W = 0.0f) { x = X; y = Y; z = Z; w = W; }
	float x, y, z, w;
};

struct GraphicsTest
{
	GraphicsTest()
		: screenWidth(1280),
		  screenHeight(720),
			fullscreen(false),
			debugDevice(true)
	{
	}

	bool Init(int argc, char **argv);

	int screenWidth;
	int screenHeight;
	bool fullscreen;
	bool debugDevice;
};

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

#define TEST_ERROR(fmt, ...) do { fprintf(stdout, "Error: " fmt, __VA_ARGS__); fflush(stdout); } while(0)
#define TEST_WARN(fmt, ...) do { fprintf(stdout, "Warning: " fmt, __VA_ARGS__); fflush(stdout); } while(0)
#define TEST_FATAL(fmt, ...) do { fprintf(stdout, "Fatal Error: " fmt, __VA_ARGS__); fflush(stdout); exit(0); } while(0)
#define TEST_UNIMPLEMENTED(fmt, ...) do { fprintf(stdout, "Unimplemented: " fmt, __VA_ARGS__); fflush(stdout); exit(0); } while(0)