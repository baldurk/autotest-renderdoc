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
  int16_t pos[3];
  uint16_t col[4];
  double uv[3];
};

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
layout(location = 2) in dvec3 UV;

layout(location = 0) out v2f vertOut;

void main()
{
	vertOut.pos = vec4(Position.xyz, 1);
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xyz, 1);
}

)EOSHADER";

std::string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col/65535.0f;
}

)EOSHADER";

struct impl : VulkanGraphicsTest
{
  int main(int argc, char **argv);

  vk::PipelineLayout layout;
  vk::RenderPass renderPass;
  std::vector<vk::Framebuffer> framebuffer;
  vk::Pipeline pipe;

  AllocatedBuffer vb;

  ~impl()
  {
    if(device)
    {
      device.destroyPipelineLayout(layout);
      device.destroyRenderPass(renderPass);
      for(vk::Framebuffer fb : framebuffer)
        device.destroyFramebuffer(fb);
      device.destroyPipeline(pipe);
    }

    vb.destroy();
  }
};

int impl::main(int argc, char **argv)
{
  features.shaderFloat64 = true;

  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  vk::ShaderModule vert = CompileGlslToSpv(common + vertex, shaderc_vertex_shader, "tri.vert");
  vk::ShaderModule frag = CompileGlslToSpv(common + pixel, shaderc_fragment_shader, "tri.frag");

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
      vk::VertexInputAttributeDescription(0, 0, vk::Format::eR16G16B16Snorm, offsetof(a2v, pos)));
  pipeCreate.attrs.push_back(vk::VertexInputAttributeDescription(
      1, 0, vk::Format::eR16G16B16A16Uscaled, offsetof(a2v, col)));
  pipeCreate.attrs.push_back(
      vk::VertexInputAttributeDescription(2, 0, vk::Format::eR64G64B64Sfloat, offsetof(a2v, uv)));

  pipeCreate.addShader(vert, vk::ShaderStageFlagBits::eVertex);
  pipeCreate.addShader(frag, vk::ShaderStageFlagBits::eFragment);

  ResultChecker(pipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  device.destroyShaderModule(vert);
  device.destroyShaderModule(frag);

  a2v triangle[] = {
      {
          {-16000, 16000, 0}, {51515, 2945, 5893, 492}, {8.2645198430, 1.8813003880, -3.96710837683597},
      },
      {
          {0, -16000, 0}, {1786, 32356, 8394, 1835}, {1.646793901, 6.86148531, -1.19476386246190},
      },
      {
          {16000, 16000, 0}, {8523, 9924, 49512, 3942}, {5.206423972, 9.58934003, -5.408522446462},
      },
  };

  vb.create(allocator,
            vk::BufferCreateInfo({}, sizeof(triangle), vk::BufferUsageFlagBits::eVertexBuffer |
                                                           vk::BufferUsageFlagBits::eTransferDst),
            VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  void *ptr = vb.map();
  memcpy(ptr, triangle, sizeof(triangle));
  vb.unmap();

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

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipe);
    cmd.setViewport(0, {viewport});
    cmd.setScissor(0, {scissor});
    cmd.bindVertexBuffers(0, {vb.buffer}, {0});
    cmd.draw(3, 1, 0, 0);

    cmd.endRenderPass();

    FinishUsingBackbuffer(cmd, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eGeneral);

    cmd.end();

    Submit(0, 1, {cmd});

    Present();
  }

  return 0;
}
};    // anonymous namespace

REGISTER_TEST("VK", "Awkward_Triangle",
              "Draws a triangle but using vertex buffers in formats that only support VBs and not "
              "any other type of buffer use (i.e. requiring manual decode)");