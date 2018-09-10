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

std::string glslpixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

layout(set = 0, binding = 0, std140) uniform constsbuf
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

std::string hlslpixel = R"EOSHADER(

layout(set = 0, binding = 0) cbuffer consts
{
  // dummy* entries are just to 'reset' packing to avoid pollution between tests

  float4 a;                               // basic float4 = {0, 1, 2, 3}
  float3 b;                               // should have a padding word at the end = {4, 5, 6}, <7>

  float2 c; float2 d;                     // should be packed together = {8, 9}, {10, 11}
  float e; float3 f;                      // should be packed together = 12, {13, 14, 15}
  float g; float2 h; float i;             // should be packed together = 16, {17, 18}, 19
  float j; float2 k;                      // should have a padding word at the end = 20, {21, 22}, <23>
  float2 l; float m;                      // should have a padding word at the end = {24, 25}, 26, <27>

  float n[4];                             // should cover 4 float4s = 28, <29..31>, 32, <33..35>, 36, <37..39>, 40
  float4 dummy1;

  float o[4];                             // should cover 4 float4s = 48, <..>, 52, <..>, 56, <..>, 60
  float p;                                // can't be packed in with above array = 64, <65, 66, 67>
  float4 dummy2;
  float4 gldummy;

  // HLSL majorness is flipped to match column-major SPIR-V with row-major HLSL.
  // This means column major declared matrices will show up as row major in any reflection and SPIR-V
  // it also means that dimensions are flipped, so a float3x4 is declared as a float4x3, and a 'row'
  // is really a column, and vice-versa a 'column' is really a row.

  column_major float4x4 q;                // should cover 4 float4s.
                                          // row1: {76, 77, 78, 79}
                                          // row2: {80, 81, 82, 83}
                                          // row3: {84, 85, 86, 87}
                                          // row3: {88, 89, 90, 91}
  row_major float4x4 r;                   // should cover 4 float4s
                                          // row0: {92, 96, 100, 104}
                                          // row1: {93, 97, 101, 105}
                                          // row2: {94, 98, 102, 106}
                                          // row3: {95, 99, 103, 107}

  column_major float3x4 s;                // covers 4 float4s with padding at end of each 'row'
                                          // row0: {108, 109, 110}, <111>
                                          // row1: {112, 113, 114}, <115>
                                          // row2: {116, 117, 118}, <119>
                                          // row3: {120, 121, 122}, <123>
  float4 dummy3;
  row_major float3x4 t;                   // covers 3 float4s with no padding
                                          // row0: {128, 132, 136}
                                          // row1: {129, 133, 137}
                                          // row2: {130, 134, 138}
                                          // row3: {131, 135, 139}
  float4 dummy4;

  column_major float2x3 u;                // covers 3 float4s with padding at end of each 'row' (but not 'column')
                                          // row0: {144, 145}, <146, 147>
                                          // row1: {148, 149}, <150, 151>
                                          // row2: {152, 153}, <154, 155>
  float4 dummy5;
  row_major float2x3 v;                   // covers 2 float4s with padding at end of each 'column' (but not 'row')
                                          // row0: {160, 164}
                                          // row1: {161, 165}
                                          // row2: {162, 166}
                                          //       <163, 167>
  float4 dummy6;

  column_major float2x2 w;                // covers 2 float4s with padding at end of each 'row' (but not 'column')
                                          // row0: {172, 173}, <174, 175>
                                          // row1: {176, 177}, <178, 179>
  float4 dummy7;
  row_major float2x2 x;                   // covers 2 float4s with padding at end of each 'column' (but not 'row')
                                          // row0: {184, 188}
                                          // row1: {185, 189}
                                          //       <186, 190>
                                          //       <187, 191>
  float4 dummy8;

  row_major float2x2 y;                   // covers the same as above, proving z doesn't overlap
                                          // row0: {196, 200}
                                          // row1: {197, 201}
                                          //       <198, 202>
                                          //       <199, 203>
  float z;                                // doesn't overlap in final row = 204, <205, 206, 207>

  row_major float4x1 aa;                  // covers 4 vec4s with maximum padding
                                          // row0: {208, 212, 216, 220}
                                          //       <209, 213, 217, 221>
                                          //       <210, 214, 218, 222>
                                          //       <211, 215, 219, 223>

  column_major float4x1 ab;               // covers 1 float4 (equivalent to a plain float4 after row/column swap)
                                          // row0: {224, 225, 226, 227}

  float4 test;                            // {228, 229, 230, 231}
};

float4 main() : SV_Target0
{
	return test;
}

)EOSHADER";

struct impl : VulkanGraphicsTest
{
  int main(int argc, char **argv);

  vk::PipelineLayout layout;
  vk::ImageView imgview;
  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
  vk::Pipeline glslpipe;
  vk::Pipeline hlslpipe;

  vk::DescriptorSetLayout setlayout;
  vk::DescriptorPool descpool;

  AllocatedImage img;
  AllocatedBuffer vb;
  AllocatedBuffer cb;

  ~impl()
  {
    if(device)
    {
      device.destroyPipelineLayout(layout);
      device.destroyImageView(imgview);
      device.destroyRenderPass(renderPass);
      device.destroyFramebuffer(framebuffer);
      device.destroyPipeline(glslpipe);
      device.destroyPipeline(hlslpipe);
      device.destroyDescriptorSetLayout(setlayout);
      device.destroyDescriptorPool(descpool);
    }

    img.destroy();
    vb.destroy();
    cb.destroy();
  }
};

int impl::main(int argc, char **argv)
{
  // initialise, create window, create context, etc
  if(!Init(argc, argv))
    return 3;

  vk::ShaderModule vert =
      CompileShaderModule(common + vertex, ShaderLang::glsl, ShaderStage::vert, "main");
  vk::ShaderModule glslfrag =
      CompileShaderModule(common + glslpixel, ShaderLang::glsl, ShaderStage::frag, "main");
  vk::ShaderModule hlslfrag =
      CompileShaderModule(common + hlslpixel, ShaderLang::hlsl, ShaderStage::frag, "main");

  if(!vert || !glslfrag || !hlslfrag)
    return 4;

  vk::DescriptorSetLayoutBinding bindings[] = {
      {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment},
  };
  ResultChecker(setlayout) = device.createDescriptorSetLayout({{}, 1, bindings});

  ResultChecker(layout) = device.createPipelineLayout({{}, 1, &setlayout});

  img.create(allocator,
             vk::ImageCreateInfo({}, vk::ImageType::e2D, vk::Format::eR32G32B32A32Sfloat,
                                 vk::Extent3D(scissor.extent.width, scissor.extent.height, 1), 1, 1,
                                 vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                                 vk::ImageUsageFlagBits::eColorAttachment),
             VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_GPU_ONLY}));

  ResultChecker(imgview) = device.createImageView({
      {},
      img.image,
      vk::ImageViewType::e2D,
      vk::Format::eR32G32B32A32Sfloat,
      vk::ComponentMapping(),
      vk::ImageSubresourceRange(),
  });

  RenderPassCreator rpCreate;

  rpCreate.atts.push_back(vk::AttachmentDescription()
                              .setFormat(vk::Format::eR32G32B32A32Sfloat)
                              .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(vk::ImageLayout::eGeneral));

  rpCreate.addSub({vk::AttachmentReference(0, vk::ImageLayout::eGeneral)});

  ResultChecker(renderPass) = device.createRenderPass(rpCreate.bake());

  ResultChecker(framebuffer) = device.createFramebuffer(vk::FramebufferCreateInfo(
      {}, renderPass, 1, &imgview, scissor.extent.width, scissor.extent.height, 1));

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
  pipeCreate.addShader(glslfrag, vk::ShaderStageFlagBits::eFragment);

  ResultChecker(glslpipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  pipeCreate.clearShaders();
  pipeCreate.addShader(vert, vk::ShaderStageFlagBits::eVertex);
  pipeCreate.addShader(hlslfrag, vk::ShaderStageFlagBits::eFragment);

  ResultChecker(hlslpipe) = device.createGraphicsPipeline(vk::PipelineCache(), pipeCreate.bake());

  device.destroyShaderModule(vert);
  device.destroyShaderModule(hlslfrag);
  device.destroyShaderModule(glslfrag);

  a2v triangle[] = {
      {
          Vec3f(-0.5f, 0.5f, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), Vec2f(0.0f, 0.0f),
      },
      {
          Vec3f(0.0f, -0.5f, 0.0f), Vec4f(0.0f, 1.0f, 0.0f, 1.0f), Vec2f(0.0f, 1.0f),
      },
      {
          Vec3f(0.5f, 0.5f, 0.0f), Vec4f(0.0f, 0.0f, 1.0f, 1.0f), Vec2f(1.0f, 0.0f),
      },
  };

  vb.create(allocator,
            vk::BufferCreateInfo({}, sizeof(triangle), vk::BufferUsageFlagBits::eVertexBuffer |
                                                           vk::BufferUsageFlagBits::eTransferDst),
            VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  void *ptr = vb.map();
  memcpy(ptr, triangle, sizeof(triangle));
  vb.unmap();

  Vec4f cbufferdata[256];

  for(int i = 0; i < 256; i++)
    cbufferdata[i] = Vec4f(float(i * 4 + 0), float(i * 4 + 1), float(i * 4 + 2), float(i * 4 + 3));

  cb.create(allocator,
            vk::BufferCreateInfo({}, sizeof(cbufferdata), vk::BufferUsageFlagBits::eUniformBuffer |
                                                              vk::BufferUsageFlagBits::eTransferDst),
            VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

  ptr = cb.map();
  memcpy(ptr, cbufferdata, sizeof(cbufferdata));
  cb.unmap();

  vk::DescriptorPoolSize poolSizes[] = {vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1)};
  ResultChecker(descpool) = device.createDescriptorPool({{}, 1, 1, poolSizes});

  std::vector<vk::DescriptorSet> descsets;
  ResultChecker(descsets) = device.allocateDescriptorSets({descpool, 1, &setlayout});

  vk::DescriptorBufferInfo bufInfo(cb.buffer, 0, VK_WHOLE_SIZE);
  device.updateDescriptorSets(
      {
          vk::WriteDescriptorSet(descsets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, NULL,
                                 &bufInfo, NULL),
      },
      {});

  while(Running())
  {
    vk::CommandBuffer cmd = GetCommandBuffer();

    cmd.begin(vk::CommandBufferBeginInfo());

    vk::Image img =
        StartUsingBackbuffer(cmd, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eGeneral);

    cmd.clearColorImage(img, vk::ImageLayout::eGeneral,
                        std::array<float, 4>({0.4f, 0.5f, 0.6f, 1.0f}),
                        {vk::ImageSubresourceRange()});

    cmd.beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffer, scissor),
                        vk::SubpassContents::eInline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, {descsets[0]}, {});
    cmd.setViewport(0, {viewport});
    cmd.setScissor(0, {scissor});
    cmd.bindVertexBuffers(0, {vb.buffer}, {0});

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, glslpipe);
    cmd.draw(3, 1, 0, 0);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, hlslpipe);
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

REGISTER_TEST("VK", "CBuffer_Zoo",
              "Tests every kind of constant that can be in a cbuffer to make sure it's decoded "
              "correctly");