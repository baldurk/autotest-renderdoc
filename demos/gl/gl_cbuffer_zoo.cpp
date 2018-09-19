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

#include "gl_test.h"

struct GL_CBuffer_Zoo : OpenGLGraphicsTest
{
  static constexpr char *Description =
      "Tests every kind of constant that can be in a cbuffer to make sure it's decoded "
      "correctly";

  std::string common = R"EOSHADER(

#version 420 core

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
};

)EOSHADER";

  std::string vertex = R"EOSHADER(

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 UV;

layout(location = 0) out v2f vertOut;

void main()
{
	vertOut.pos = vec4(Position.xyz, 1);
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xy, 0, 1);
}

)EOSHADER";

  std::string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

layout(binding = 0, std140) uniform constsbuf
{
  // dummy* entries are just to 'reset' packing to avoid pollution between tests

  vec4 a;                               // basic vec4 = {0, 1, 2, 3}
  vec3 b;                               // should have a padding word at the end = {4, 5, 6}, <7>

  vec2 c; vec2 d;                       // should be packed together = {8, 9}, {10, 11}
  float e; vec3 f;                      // can't be packed together = 12, <13, 14, 15>, {16, 17, 18}, <19>
  vec4 dummy0;
  float j; vec2 k;                      // should have a padding word before the vec2 = 24, <25>, {26, 27}
  vec2 l; float m;                      // should have a padding word at the end = {28, 29}, 30, <31>

  float n[4];                           // should cover 4 vec4s = 32, <33..35>, 36, <37..39>, 40, <41..43>, 44
  vec4 dummy1;

  float o[4];                           // should cover 4 vec4s = 52, <53..55>, 56, <57..59>, 60, <61..63>, 64
  float p;                              // can't be packed in with above array = 68, <69, 70, 71>
  vec4 dummy2;

  layout(column_major) mat4x4 q;        // should cover 4 vec4s.
                                        // row0: {76, 80, 84, 88}
                                        // row1: {77, 81, 85, 89}
                                        // row2: {78, 82, 86, 90}
                                        // row3: {79, 83, 87, 91}
  layout(row_major) mat4x4 r;           // should cover 4 vec4s
                                        // row0: {92, 93, 94, 95}
                                        // row1: {96, 97, 98, 99}
                                        // row2: {100, 101, 102, 103}
                                        // row3: {104, 105, 106, 107}

  layout(column_major) mat4x3 s;        // covers 4 vec4s with padding at end of each column
                                        // row0: {108, 112, 116, 120}
                                        // row1: {109, 113, 117, 121}
                                        // row2: {110, 114, 118, 122}
                                        //       <111, 115, 119, 123>
  vec4 dummy3;
  layout(row_major) mat4x3 t;           // covers 3 vec4s with no padding
                                        // row0: {128, 129, 130, 131}
                                        // row1: {132, 133, 134, 135}
                                        // row2: {136, 137, 138, 139}
  vec4 dummy4;

  layout(column_major) mat3x2 u;        // covers 3 vec4s with padding at end of each column (but not row)
                                        // row0: {144, 148, 152}
                                        // row1: {145, 149, 153}
                                        //       <146, 150, 154>
                                        //       <147, 151, 155>
  vec4 dummy5;
  layout(row_major) mat3x2 v;           // covers 2 vec4s with padding at end of each row (but not column)
                                        // row0: {160, 161, 162}, <163>
                                        // row1: {164, 165, 166}, <167>
  vec4 dummy6;

  layout(column_major) mat2x2 w;        // covers 2 vec4s with padding at end of each column (but not row)
                                        // row0: {172, 176}
                                        // row1: {173, 177}
                                        //       <174, 178>
                                        //       <175, 179>
  vec4 dummy7;
  layout(row_major) mat2x2 x;           // covers 2 vec4s with padding at end of each row (but not column)
                                        // row0: {184, 185}, <186, 187>
                                        // row1: {188, 189}, <190, 191>
  vec4 dummy8;

  layout(row_major) mat2x2 y;           // covers the same as above, and checks z doesn't overlap
                                        // row0: {196, 197}, <198, 199>
                                        // row1: {200, 201}, <202, 203>
  float z;                              // can't overlap = 204, <205, 206, 207>

  // GL Doesn't have single-column matrices
/*
  layout(row_major) mat1x4 aa;          // covers 4 vec4s with maximum padding
                                        // row0: {208}, <209, 210, 211>
                                        // row1: {212}, <213, 214, 215>
                                        // row2: {216}, <217, 218, 219>
                                        // row3: {220}, <221, 222, 223>

  layout(column_major) mat1x4 ab;       // covers 1 vec4 (equivalent to a plain vec4)
                                        // row0: {224}
                                        // row1: {225}
                                        // row2: {226}
                                        // row3: {227}
*/
  vec4 dummy9[5];

  vec4 test;                            // {228, 229, 230, 231}
};

void main()
{
  // we need to ref all of the variables we want to include to force GL to include them :(.
  float blah = a.x + b.x + c.x + d.x + e.x + f.x + j.x + k.x + l.x + m.x;
  blah += n[0] + o[0] + p.x;
  blah += q[0].x + r[0].x + s[0].x + t[0].x + u[0].x + v[0].x + w[0].x + x[0].x + y[0].x + z;
  blah *= 0.0000001f;
  Color = blah + test;
}

)EOSHADER";

  int main(int argc, char **argv)
  {
    // initialise, create window, create context, etc
    if(!Init(argc, argv))
      return 3;

    GLuint vao = MakeVAO();
    glBindVertexArray(vao);

    GLuint vb = MakeBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(DefaultTri), DefaultTri, 0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V), (void *)(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V), (void *)(sizeof(Vec3f)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(DefaultA2V),
                          (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    GLuint program = MakeProgram(common + vertex, common + pixel);
    glObjectLabel(GL_PROGRAM, program, -1, "Full program");

    Vec4f cbufferdata[256];

    for(int i = 0; i < 256; i++)
      cbufferdata[i] = Vec4f(float(i * 4 + 0), float(i * 4 + 1), float(i * 4 + 2), float(i * 4 + 3));

    GLuint cb = MakeBuffer();
    glBindBuffer(GL_UNIFORM_BUFFER, cb);
    glBufferStorage(GL_UNIFORM_BUFFER, sizeof(cbufferdata), cbufferdata, GL_MAP_WRITE_BIT);

    GLuint fbo = MakeFBO();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Color render texture
    GLuint colattach = MakeTexture();

    glBindTexture(GL_TEXTURE_2D, colattach);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, screenWidth, screenHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colattach, 0);

    while(Running())
    {
      float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
      glClearBufferfv(GL_COLOR, 0, col);

      glBindVertexArray(vao);

      glBindBufferBase(GL_UNIFORM_BUFFER, 0, cb);

      glUseProgram(program);

      glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

      glDrawArrays(GL_TRIANGLES, 0, 3);

      Present();
    }

    return 0;
  }
};

REGISTER_TEST(GL_CBuffer_Zoo);