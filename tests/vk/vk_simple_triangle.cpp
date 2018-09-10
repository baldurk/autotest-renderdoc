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

struct VK_Simple_Triangle : VulkanGraphicsTest
{
  static constexpr char *Description =
      "Just draws a simple triangle, using normal pipeline. Basic test that can be used "
      "for any dead-simple tests that don't require any particular API use";

  std::string common = R"EOSHADER(

#version 420 core

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
};

)EOSHADER";

  const std::string vertex = R"EOSHADER(

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

  const std::string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col;
}

)EOSHADER";

  int main(int argc, char **argv)
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

    vk::PipelineLayout layout;
    ResultChecker(layout) = device.createPipelineLayout({});

    RenderPassCreator rpCreate;

    rpCreate.atts.push_back(vk::AttachmentDescription()
                                .setFormat(swapFormat)
                                .setInitialLayout(vk::ImageLayout::eGeneral)
                                .setFinalLayout(vk::ImageLayout::eGeneral));

    rpCreate.addSub({vk::AttachmentReference(0, vk::ImageLayout::eGeneral)});

    vk::RenderPass renderPass;
    ResultChecker(renderPass) = device.createRenderPass(rpCreate.bake());

    std::vector<vk::Framebuffer> framebuffer;
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
        vk::VertexInputBindingDescription(0, sizeof(DefaultA2V), vk::VertexInputRate::eVertex));
    pipeCreate.attrs.push_back(vk::VertexInputAttributeDescription(0, 0, formatof(DefaultA2V::pos),
                                                                   offsetof(DefaultA2V, pos)));
    pipeCreate.attrs.push_back(vk::VertexInputAttributeDescription(1, 0, formatof(DefaultA2V::col),
                                                                   offsetof(DefaultA2V, col)));
    pipeCreate.attrs.push_back(vk::VertexInputAttributeDescription(2, 0, formatof(DefaultA2V::uv),
                                                                   offsetof(DefaultA2V, uv)));

    pipeCreate.addShader(vert, vk::ShaderStageFlagBits::eVertex);
    pipeCreate.addShader(frag, vk::ShaderStageFlagBits::eFragment);

    vk::Pipeline pipe;
    ResultChecker(pipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

    device.destroyShaderModule(vert);
    device.destroyShaderModule(frag);

    AllocatedBuffer vb;
    vb.create(allocator,
              vk::BufferCreateInfo({}, sizeof(DefaultTri), vk::BufferUsageFlagBits::eVertexBuffer |
                                                               vk::BufferUsageFlagBits::eTransferDst),
              VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

    void *ptr = vb.map();
    memcpy(ptr, DefaultTri, sizeof(DefaultTri));
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

    device.destroyPipelineLayout(layout);
    device.destroyRenderPass(renderPass);
    for(vk::Framebuffer fb : framebuffer)
      device.destroyFramebuffer(fb);
    device.destroyPipeline(pipe);

    vb.destroy();

    return 0;
  }
};

REGISTER_TEST(VK_Simple_Triangle);