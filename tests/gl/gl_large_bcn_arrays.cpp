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

#include <time.h>
#include "../gl_common.h"

namespace
{
struct a2v
{
  Vec3f pos;
  Vec4f col;
  Vec2f uv;
};

string common = R"EOSHADER(

#version 420 core

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
};

)EOSHADER";

string vertex = R"EOSHADER(

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

string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col;
}

)EOSHADER";

struct impl : OpenGLGraphicsTest
{
  int main(int argc, char **argv);

  GLuint vao;
  GLuint vb;

  GLuint program;
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  a2v triangle[] = {
      {
          Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  vao = MakeVAO();
  glBindVertexArray(vao);

  vb = MakeBuffer();
  glBindBuffer(GL_ARRAY_BUFFER, vb);
  glBufferStorage(GL_ARRAY_BUFFER, sizeof(triangle), triangle, 0);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(0));
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(a2v), (void *)(sizeof(Vec3f)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(a2v),
                        (void *)(sizeof(Vec3f) + sizeof(Vec4f)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  program = MakeProgram(common + vertex, common + pixel);
  glObjectLabel(GL_PROGRAM, program, -1, "Full program");

  srand((unsigned int)time(NULL));

  const int width = 4;
  const int height = 4;
  const int numMips = 1;

  int activeTex = GL_TEXTURE0;

  GLuint texs[2][4];

  GLenum fmts[] = {GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_RG_RGTC2,
                   GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, GL_COMPRESSED_RGBA_BPTC_UNORM};

  for(int arraySize = 1; arraySize <= 2; arraySize++)
  {
    GLenum texbind = arraySize > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

    const char *names[] = {
        arraySize > 1 ? "BC4 array" : "BC4", arraySize > 1 ? "BC5 array" : "BC5",
        arraySize > 1 ? "BC6 array" : "BC6", arraySize > 1 ? "BC7 array" : "BC7",
    };

    for(int fmt = 0; fmt < 4; fmt++)
    {
      glActiveTexture(activeTex);
      activeTex++;

      GLuint tex = texs[arraySize - 1][fmt] = MakeTexture();

      glBindTexture(texbind, tex);
      if(texbind == GL_TEXTURE_2D_ARRAY)
        glTexStorage3D(texbind, numMips, fmts[fmt], width, height, arraySize);
      else
        glTexStorage2D(texbind, numMips, fmts[fmt], width, height);

      // force renderdoc to late-fetch the texture contents, and not serialise the
      // subimage data calls below
      for(int blah = 0; blah < 100; blah++)
        glTexParameteri(texbind, GL_TEXTURE_MAX_LEVEL, numMips - 1);

      glObjectLabel(GL_TEXTURE, tex, -1, names[fmt]);

      int w = width;
      int h = height;

      for(int mip = 0; mip < numMips; mip++)
      {
        byte *foo = new byte[w * h * arraySize];
        for(int len = 0; len < w * h * arraySize; len++)
          foo[len] = rand() & 0xff;

        GLsizei size = w * h * arraySize;
        // BC4 is 0.5 bytes per pixel
        if(fmt == 0)
          size /= 2;

        if(texbind == GL_TEXTURE_2D_ARRAY)
          glCompressedTexSubImage3D(texbind, mip, 0, 0, 0, w, h, arraySize, fmts[fmt], size, foo);
        else
          glCompressedTexSubImage2D(texbind, mip, 0, 0, w, h, fmts[fmt], size, foo);

        w = w >> 1;
        h = h >> 1;

        delete[] foo;
      }
    }
  }

// replicates the stuff that renderdoc does for saving and restoring texture contents, so that the
// bug can be investigated without involving renderdoc at all
#if 1
  for(int arraySize = 1; arraySize <= 2; arraySize++)
  {
    GLenum texbind = arraySize > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

    const char *names[] = {
        arraySize > 1 ? "replay copy of BC4 array" : "replay copy of BC4",
        arraySize > 1 ? "replay copy of BC5 array" : "replay copy of BC5",
        arraySize > 1 ? "replay copy of BC6 array" : "replay copy of BC6",
        arraySize > 1 ? "replay copy of BC7 array" : "replay copy of BC7",
    };

    const char *preparedNames[] = {
        arraySize > 1 ? "prepared copy of BC4 array" : "prepared copy of BC4",
        arraySize > 1 ? "prepared copy of BC5 array" : "prepared copy of BC5",
        arraySize > 1 ? "prepared copy of BC6 array" : "prepared copy of BC6",
        arraySize > 1 ? "prepared copy of BC7 array" : "prepared copy of BC7",
    };

    const char *initNames[] = {
        arraySize > 1 ? "init contents of BC4 array" : "init contents of BC4",
        arraySize > 1 ? "init contents of BC5 array" : "init contents of BC5",
        arraySize > 1 ? "init contents of BC6 array" : "init contents of BC6",
        arraySize > 1 ? "init contents of BC7 array" : "init contents of BC7",
    };

    for(int fmt = 0; fmt < 4; fmt++)
    {
      // make a copy to save the state at the start of the frame
      GLuint prepared = MakeTexture();
      glBindTexture(texbind, prepared);

      int w = width;
      int h = height;

      // reserve the texture
      for(int mip = 0; mip < numMips; mip++)
      {
        if(arraySize > 1)
          glTextureImage3DEXT(prepared, texbind, mip, fmts[fmt], w, h, arraySize, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL);
        else
          glTextureImage2DEXT(prepared, texbind, mip, fmts[fmt], w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                              NULL);

        w = w >> 1;
        h = h >> 1;
      }

      glObjectLabel(GL_TEXTURE, prepared, -1, preparedNames[fmt]);

      // ensure texture is mipmap complete
      GLint maxlevel = numMips - 1;
      glTextureParameterivEXT(prepared, texbind, GL_TEXTURE_MAX_LEVEL, &maxlevel);

      // copy the actual data
      w = width;
      h = height;

      for(int mip = 0; mip < numMips; mip++)
      {
        glCopyImageSubData(texs[arraySize - 1][fmt], texbind, mip, 0, 0, 0, prepared, texbind, mip,
                           0, 0, 0, w, h, arraySize);

        w = w >> 1;
        h = h >> 1;
      }

      // ...frame capture happens here...

      // readback the data
      w = width;
      h = height;

      byte buf[numMips][32] = {0xfe};

      for(int mip = 0; mip < numMips; mip++)
      {
        GLsizei size = w * h * arraySize;
        // BC4 is 0.5 bytes per pixel
        if(fmt == 0)
          size /= 2;

        glGetCompressedTextureImageEXT(prepared, texbind, mip, buf[mip]);

        w = w >> 1;
        h = h >> 1;
      }

      // buf is serialised to disk in the capture now

      // on replay create the replay-side version of the original texture
      GLuint replayTex = MakeTexture();

      glBindTexture(texbind, replayTex);
      if(texbind == GL_TEXTURE_2D_ARRAY)
        glTextureStorage3DEXT(replayTex, texbind, numMips, fmts[fmt], width, height, arraySize);
      else
        glTextureStorage2DEXT(replayTex, texbind, numMips, fmts[fmt], width, height);

      glObjectLabel(GL_TEXTURE, replayTex, -1, names[fmt]);

      // now create a new texture to hold the initial contents
      GLuint initContents = MakeTexture();
      glBindTexture(texbind, initContents);

      glObjectLabel(GL_TEXTURE, replayTex, -1, initNames[fmt]);

      // reserve the texture
      w = width;
      h = height;

      for(int mip = 0; mip < numMips; mip++)
      {
        if(arraySize > 1)
          glTextureImage3DEXT(initContents, texbind, mip, fmts[fmt], w, h, arraySize, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL);
        else
          glTextureImage2DEXT(initContents, texbind, mip, fmts[fmt], w, h, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, NULL);

        w = w >> 1;
        h = h >> 1;
      }

      glTextureParameterivEXT(initContents, texbind, GL_TEXTURE_MAX_LEVEL, &maxlevel);

      // upload the stored contents
      w = width;
      h = height;

      for(int mip = 0; mip < numMips; mip++)
      {
        GLsizei size = w * h * arraySize;
        // BC4 is 0.5 bytes per pixel
        if(fmt == 0)
          size /= 2;

        if(arraySize > 1)
          glCompressedTextureSubImage3DEXT(initContents, texbind, mip, 0, 0, 0, w, h, arraySize,
                                           fmts[fmt], size, buf[mip]);
        else
          glCompressedTextureSubImage2DEXT(initContents, texbind, mip, 0, 0, w, h, fmts[fmt], size,
                                           buf[mip]);

        w = w >> 1;
        h = h >> 1;
      }

      // now whenever we want to restore the texture to its frame-initial state, we do a copy
      w = width;
      h = height;

      for(int mip = 0; mip < numMips; mip++)
      {
        glCopyImageSubData(initContents, texbind, mip, 0, 0, 0, replayTex, texbind, mip, 0, 0, 0, w,
                           h, arraySize);

        w = w >> 1;
        h = h >> 1;
      }

      // replayTex should now have identical contents to the original tex
    }
  }
#endif

  while(Running())
  {
    float col[] = {0.4f, 0.5f, 0.6f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, col);

    glBindVertexArray(vao);

    glUseProgram(program);

    glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenHeight));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    Present();
  }

  return 0;
}

};    // anonymous namespace

int GL_Large_BCn_Arrays(int argc, char **argv)
{
  impl i;
  return i.main(argc, argv);
}