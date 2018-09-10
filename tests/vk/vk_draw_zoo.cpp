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

#include "../vk_common.h"

namespace
{
struct a2v
{
  Vec3f pos;
  Vec4f col;
  Vec2f uv;
};

std::string common = R"EOSHADER(

#version 420 core

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
  float vertidx;
  float instidx;
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
  vertOut.pos.x += 0.1f*gl_InstanceIndex - 0.5f;
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xy, 0, 1);

  vertOut.vertidx = float(gl_VertexIndex);
  vertOut.instidx = float(gl_InstanceIndex);
}

)EOSHADER";

std::string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col;
  Color.g = vertIn.vertidx/30.0f;
  Color.b = vertIn.instidx/10.0f;
}

)EOSHADER";

struct impl : VulkanGraphicsTest
{
  int main(int argc, char **argv);

  vk::PipelineLayout layout;
  vk::RenderPass renderPass;
  std::vector<vk::Framebuffer> framebuffer;
  vk::Pipeline noInstPipe;
  vk::Pipeline instPipe;
  vk::Pipeline stripPipe;

  AllocatedBuffer vb1;
  AllocatedBuffer ib1;

  AllocatedBuffer vb2;

  ~impl()
  {
    if(device)
    {
      device.destroyPipelineLayout(layout);
      device.destroyRenderPass(renderPass);
      for(vk::Framebuffer fb : framebuffer)
        device.destroyFramebuffer(fb);
      device.destroyPipeline(noInstPipe);
      device.destroyPipeline(instPipe);
      device.destroyPipeline(stripPipe);
    }

    vb1.destroy();
    ib1.destroy();
    vb2.destroy();
  }
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  vk::ShaderModule vert =
      CompileShaderModule(common + vertex, ShaderLang::glsl, ShaderStage::vert, "main");
  vk::ShaderModule frag =
      CompileShaderModule(common + pixel, ShaderLang::glsl, ShaderStage::frag, "main");

  if(!vert || !frag)
    return 4;

  ResultChecker(layout) = device.createPipelineLayout({});

  RenderPassCreator rpCreate;

  rpCreate.atts.push_back(vk::AttachmentDescription()
                              .setFormat(swapFormat)
                              .setInitialLayout(vk::ImageLayout::eGeneral)
                              .setFinalLayout(vk::ImageLayout::eGeneral));

  rpCreate.addSub({vk::AttachmentReference(0, vk::ImageLayout::eGeneral)});

  ResultChecker(renderPass) = device.createRenderPass(rpCreate.bake());

  for(size_t i = 0; i < swapImageViews.size(); i++)
  {
    vk::Framebuffer fb;
    ResultChecker(fb) = device.createFramebuffer(vk::FramebufferCreateInfo(
        {}, renderPass, 1, &swapImageViews[i], scissor.extent.width, scissor.extent.height, 1));
    framebuffer.push_back(fb);
  }

  PipelineCreator pipeCreate;

  pipeCreate.layout = layout;
  pipeCreate.renderPass = renderPass;

  pipeCreate.binds.push_back(
      vk::VertexInputBindingDescription(0, sizeof(a2v), vk::VertexInputRate::eVertex));
  pipeCreate.attrs.push_back(
      vk::VertexInputAttributeDescription(0, 0, formatof(a2v::pos), offsetof(a2v, pos)));
  pipeCreate.attrs.push_back(
      vk::VertexInputAttributeDescription(1, 0, formatof(a2v::col), offsetof(a2v, col)));
  pipeCreate.attrs.push_back(
      vk::VertexInputAttributeDescription(2, 0, formatof(a2v::uv), offsetof(a2v, uv)));

  pipeCreate.addShader(vert, vk::ShaderStageFlagBits::eVertex);
  pipeCreate.addShader(frag, vk::ShaderStageFlagBits::eFragment);

  ResultChecker(noInstPipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  pipeCreate.ia.primitiveRestartEnable = true;
  pipeCreate.ia.topology = vk::PrimitiveTopology::eTriangleStrip;

  ResultChecker(stripPipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  pipeCreate.ia.primitiveRestartEnable = false;
  pipeCreate.ia.topology = vk::PrimitiveTopology::eTriangleList;

  // add an instance vertex buffer for colours
  pipeCreate.binds.push_back(
      vk::VertexInputBindingDescription(1, sizeof(Vec4f), vk::VertexInputRate::eInstance));
  pipeCreate.attrs[1].binding = 1;
  pipeCreate.attrs[1].offset = 0;

  ResultChecker(instPipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  device.destroyShaderModule(vert);
  device.destroyShaderModule(frag);

  a2v triangle[] = {
      // 0
      {
          Vec3f(-1.0f, -1.0f, -1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(-1.0f, -1.0f),
      },
      // 1, 2, 3
      {
          Vec3f(-0.5f, 0.5f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 4, 5, 6
      {
          Vec3f(-0.5f, -0.5f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, -0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 7, 8, 9
      {
          Vec3f(-0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // 10, 11, 12
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, 0.5f, 0.0f), Vec4f(0.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      // strips: 13, 14, 15, ...
      {
          Vec3f(-0.5f, 0.2f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(-0.5f, 0.0f, 0.0f), Vec4f(0.2f, 0.1f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(-0.3f, 0.2f, 0.0f), Vec4f(0.4f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(-0.3f, 0.0f, 0.0f), Vec4f(0.6f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(-0.1f, 0.2f, 0.0f), Vec4f(0.8f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(-0.1f, 0.0f, 0.0f), Vec4f(1.0f, 0.5f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.1f, 0.2f, 0.0f), Vec4f(0.0f, 0.8f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.1f, 0.0f, 0.0f), Vec4f(0.2f, 0.1f, 0.5f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.3f, 0.2f, 0.0f), Vec4f(0.4f, 0.3f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.3f, 0.0f, 0.0f), Vec4f(0.6f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.5f, 0.2f, 0.0f), Vec4f(0.8f, 0.3f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
      {
          Vec3f(0.5f, 0.0f, 0.0f), Vec4f(1.0f, 0.1f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  vb1.create(allocator,
             vk::BufferCreateInfo({}, sizeof(a2v) * 50, vk::BufferUsageFlagBits::eVertexBuffer |
                                                            vk::BufferUsageFlagBits::eTransferDst),
             VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  {
    a2v *src = (a2v *)triangle;
    a2v *dst = (a2v *)vb1.map();

    // up-pointing triangle to offset 0
    memcpy(dst + 0, triangle + 1, sizeof(a2v));
    memcpy(dst + 1, triangle + 2, sizeof(a2v));
    memcpy(dst + 2, triangle + 3, sizeof(a2v));

    // invalid vert for index 3 and 4
    memcpy(dst + 3, triangle + 0, sizeof(a2v));
    memcpy(dst + 4, triangle + 0, sizeof(a2v));

    // down-pointing triangle at offset 5
    memcpy(dst + 5, triangle + 4, sizeof(a2v));
    memcpy(dst + 6, triangle + 5, sizeof(a2v));
    memcpy(dst + 7, triangle + 6, sizeof(a2v));

    // invalid vert for 8 - 12
    memcpy(dst + 8, triangle + 0, sizeof(a2v));
    memcpy(dst + 9, triangle + 0, sizeof(a2v));
    memcpy(dst + 10, triangle + 0, sizeof(a2v));
    memcpy(dst + 11, triangle + 0, sizeof(a2v));
    memcpy(dst + 12, triangle + 0, sizeof(a2v));

    // left-pointing triangle data to offset 13
    memcpy(dst + 13, triangle + 7, sizeof(a2v));
    memcpy(dst + 14, triangle + 8, sizeof(a2v));
    memcpy(dst + 15, triangle + 9, sizeof(a2v));

    // invalid vert for 16-22
    memcpy(dst + 16, triangle + 0, sizeof(a2v));
    memcpy(dst + 17, triangle + 0, sizeof(a2v));
    memcpy(dst + 18, triangle + 0, sizeof(a2v));
    memcpy(dst + 19, triangle + 0, sizeof(a2v));
    memcpy(dst + 20, triangle + 0, sizeof(a2v));
    memcpy(dst + 21, triangle + 0, sizeof(a2v));
    memcpy(dst + 22, triangle + 0, sizeof(a2v));

    // right-pointing triangle data to offset 23
    memcpy(dst + 23, triangle + 10, sizeof(a2v));
    memcpy(dst + 24, triangle + 11, sizeof(a2v));
    memcpy(dst + 25, triangle + 12, sizeof(a2v));

    // strip after 30
    memcpy(dst + 30, triangle + 13, sizeof(a2v));
    memcpy(dst + 31, triangle + 14, sizeof(a2v));
    memcpy(dst + 32, triangle + 15, sizeof(a2v));
    memcpy(dst + 33, triangle + 16, sizeof(a2v));
    memcpy(dst + 34, triangle + 17, sizeof(a2v));
    memcpy(dst + 35, triangle + 18, sizeof(a2v));
    memcpy(dst + 36, triangle + 19, sizeof(a2v));
    memcpy(dst + 37, triangle + 20, sizeof(a2v));
    memcpy(dst + 38, triangle + 21, sizeof(a2v));
    memcpy(dst + 39, triangle + 22, sizeof(a2v));
    memcpy(dst + 40, triangle + 23, sizeof(a2v));
    memcpy(dst + 41, triangle + 24, sizeof(a2v));

    vb1.unmap();
  }

  vb2.create(allocator,
             vk::BufferCreateInfo({}, sizeof(Vec4f) * 16, vk::BufferUsageFlagBits::eVertexBuffer |
                                                              vk::BufferUsageFlagBits::eTransferDst),
             VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  {
    Vec4f *dst = (Vec4f *)vb2.map();

    memset(dst, 0, sizeof(Vec4f) * 60);

    dst[0] = Vec4f(0.0f, 0.5f, 1.0f, 1.0f);
    dst[1] = Vec4f(1.0f, 0.5f, 0.0f, 1.0f);

    dst[5] = Vec4f(1.0f, 0.5f, 0.5f, 1.0f);
    dst[6] = Vec4f(0.5f, 0.5f, 1.0f, 1.0f);

    dst[13] = Vec4f(0.1f, 0.8f, 0.3f, 1.0f);
    dst[14] = Vec4f(0.8f, 0.1f, 0.1f, 1.0f);

    vb2.unmap();
  }

  ib1.create(allocator, vk::BufferCreateInfo({}, sizeof(uint32_t) * 100,
                                             vk::BufferUsageFlagBits::eIndexBuffer |
                                                 vk::BufferUsageFlagBits::eTransferDst),
             VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  {
    uint32_t *dst = (uint32_t *)ib1.map();

    memset(dst, 0, sizeof(uint32_t) * 100);

    dst[0] = 0;
    dst[1] = 1;
    dst[2] = 2;

    dst[5] = 5;
    dst[6] = 6;
    dst[7] = 7;

    dst[13] = 63;
    dst[14] = 64;
    dst[15] = 65;

    dst[23] = 103;
    dst[24] = 104;
    dst[25] = 105;

    dst[37] = 104;
    dst[38] = 105;
    dst[39] = 106;

    dst[42] = 30;
    dst[43] = 31;
    dst[44] = 32;
    dst[45] = 33;
    dst[46] = 34;
    dst[47] = 0xffffffff;
    dst[48] = 36;
    dst[49] = 37;
    dst[50] = 38;
    dst[51] = 39;
    dst[52] = 40;
    dst[53] = 41;

    ib1.unmap();
  }

  while(Running())
  {
    vk::CommandBuffer cmd = GetCommandBuffer();

    cmd.begin(vk::CommandBufferBeginInfo());

    vk::Image img =
        StartUsingBackbuffer(cmd, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eGeneral);

    cmd.clearColorImage(img, vk::ImageLayout::eGeneral,
                        std::array<float, 4>({0.4f, 0.5f, 0.6f, 1.0f}),
                        {vk::ImageSubresourceRange()});

    cmd.beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffer[swapIndex], scissor),
                        vk::SubpassContents::eInline);

    cmd.setScissor(0, {scissor});

    vk::Viewport vp = viewport;
    vp.width = 128.0f;
    vp.height = 128.0f;

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, noInstPipe);

    ///////////////////////////////////////////////////
    // non-indexed, non-instanced

    // basic test
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.draw(3, 1, 0, 0);
    vp.x += vp.width;

    // test with vertex offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.draw(3, 1, 5, 0);
    vp.x += vp.width;

    // test with vertex offset and vbuffer offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {5 * sizeof(a2v)});
    cmd.draw(3, 1, 8, 0);
    vp.x += vp.width;

    // adjust to next row
    vp.x = 0.0f;
    vp.y += vp.height;

    ///////////////////////////////////////////////////
    // indexed, non-instanced

    // basic test
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 1, 0, 0, 0);
    vp.x += vp.width;

    // test with first index
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 1, 5, 0, 0);
    vp.x += vp.width;

    // test with first index and vertex offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 1, 13, -50, 0);
    vp.x += vp.width;

    // test with first index and vertex offset and vbuffer offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {10 * sizeof(a2v)});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 1, 23, -100, 0);
    vp.x += vp.width;

    // test with first index and vertex offset and vbuffer offset and ibuffer offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {19 * sizeof(a2v)});
    cmd.bindIndexBuffer(ib1.buffer, 14 * sizeof(uint32_t), vk::IndexType::eUint32);
    cmd.drawIndexed(3, 1, 23, -100, 0);
    vp.x += vp.width;

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, stripPipe);

    // indexed strip with primitive restart
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer}, {0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(12, 1, 42, 0, 0);
    vp.x += vp.width;

    // adjust to next row
    vp.x = 0.0f;
    vp.y += vp.height;

    ///////////////////////////////////////////////////
    // non-indexed, instanced

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, instPipe);

    // basic test
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {0, 0});
    cmd.draw(3, 2, 0, 0);
    vp.x += vp.width;

    // basic test with first instance
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {5 * sizeof(a2v), 0});
    cmd.draw(3, 2, 0, 5);
    vp.x += vp.width;

    // basic test with first instance and instance buffer offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {13 * sizeof(a2v), 8 * sizeof(Vec4f)});
    cmd.draw(3, 2, 0, 5);
    vp.x += vp.width;

    // adjust to next row
    vp.x = 0.0f;
    vp.y += vp.height;

    ///////////////////////////////////////////////////
    // indexed, instanced

    // basic test
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {0, 0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 2, 5, 0, 0);
    vp.x += vp.width;

    // basic test with first instance
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {0, 0});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 2, 13, -50, 5);
    vp.x += vp.width;

    // basic test with first instance and instance buffer offset
    cmd.setViewport(0, {vp});
    cmd.bindVertexBuffers(0, {vb1.buffer, vb2.buffer}, {0, 8 * sizeof(Vec4f)});
    cmd.bindIndexBuffer(ib1.buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(3, 2, 23, -80, 5);
    vp.x += vp.width;

    cmd.endRenderPass();

    FinishUsingBackbuffer(cmd, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eGeneral);

    cmd.end();

    Submit(0, 1, {cmd});

    Present();
  }

  return 0;
}
};    // anonymous namespace

REGISTER_TEST("VK", "Draw_Zoo", "Draws several variants using different vertex/index offsets.");