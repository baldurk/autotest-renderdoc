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

#pragma once

#include "test_common.h"

#define VULKAN_HPP_ASSERT(cond) TEST_ASSERT(cond, "vulkan.hpp")

#define VULKAN_HPP_NO_EXCEPTIONS
// don't actually care about this, but it doesn't compile otherwise
#define VULKAN_HPP_NO_SMART_HANDLE

#include "volk/volk.h"

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vk_mem_alloc.h>

#include <list>
#include <set>
#include <vector>

struct PipelineCreator
{
  PipelineCreator();

  inline void clearShaders() { stages.clear(); }
  inline void addShader(vk::ShaderModule module, vk::ShaderStageFlagBits stage,
                        const char *entry = "main")
  {
    stages.push_back(vk::PipelineShaderStageCreateInfo({}, stage, module, entry));
  }

  vk::GraphicsPipelineCreateInfo bake();

  vk::PipelineLayout layout;
  vk::RenderPass renderPass;

  std::vector<vk::VertexInputAttributeDescription> attrs;
  std::vector<vk::VertexInputBindingDescription> binds;

  std::vector<vk::DynamicState> dyns;
  std::vector<vk::PipelineColorBlendAttachmentState> blends;

  vk::PipelineVertexInputStateCreateInfo vi;
  vk::PipelineInputAssemblyStateCreateInfo ia;
  vk::PipelineViewportStateCreateInfo vp;
  vk::PipelineMultisampleStateCreateInfo msaa;
  vk::PipelineRasterizationStateCreateInfo rs;
  vk::PipelineDepthStencilStateCreateInfo ds;
  vk::PipelineColorBlendStateCreateInfo cb;

private:
  vk::PipelineDynamicStateCreateInfo dynamic;
  std::vector<vk::PipelineShaderStageCreateInfo> stages;
};

struct RenderPassCreator
{
  vk::RenderPassCreateInfo bake()
  {
    return vk::RenderPassCreateInfo()
        .setPSubpasses(subs.data())
        .setSubpassCount((uint32_t)subs.size())
        .setPDependencies(deps.data())
        .setDependencyCount((uint32_t)deps.size())
        .setPAttachments(atts.data())
        .setAttachmentCount((uint32_t)atts.size());
  }

  void addSub(const std::vector<vk::AttachmentReference> &color,
              uint32_t depthAttachment = VK_ATTACHMENT_UNUSED,
              vk::ImageLayout depthLayout = vk::ImageLayout::eUndefined)
  {
    colors.push_back(color);
    depths.push_back(vk::AttachmentReference(depthAttachment, depthLayout));

    subs.push_back(vk::SubpassDescription(
        {}, vk::PipelineBindPoint::eGraphics, 0, NULL, (uint32_t)color.size(), colors.back().data(),
        NULL, depthAttachment == VK_ATTACHMENT_UNUSED ? &depths.back() : NULL));
  }

  std::vector<vk::AttachmentDescription> atts;
  std::vector<vk::SubpassDependency> deps;
  std::vector<vk::SubpassDescription> subs;

private:
  std::list<std::vector<vk::AttachmentReference>> colors;
  std::list<vk::AttachmentReference> depths = {};
};

struct AllocatedBuffer
{
  VmaAllocator allocator;
  vk::Buffer buffer;
  VmaAllocation alloc;

  void create(VmaAllocator allocator, const vk::BufferCreateInfo &bufInfo,
              const VmaAllocationCreateInfo &allocInfo)
  {
    this->allocator = allocator;
    VkBuffer buf;
    vmaCreateBuffer(allocator, &(const VkBufferCreateInfo &)bufInfo, &allocInfo, &buf, &alloc, NULL);
    buffer = vk::Buffer(buf);
  }

  byte *map()
  {
    byte *ret = NULL;
    VkResult vkr = vmaMapMemory(allocator, alloc, (void **)&ret);

    if(vkr != VK_SUCCESS)
      return NULL;

    return ret;
  }

  void unmap() { vmaUnmapMemory(allocator, alloc); }
  void destroy() { vmaDestroyBuffer(allocator, (VkBuffer)buffer, alloc); }
};

struct AllocatedImage
{
  VmaAllocator allocator;
  vk::Image image;
  VmaAllocation alloc;

  void create(VmaAllocator allocator, const vk::ImageCreateInfo &imgInfo,
              const VmaAllocationCreateInfo &allocInfo)
  {
    this->allocator = allocator;
    VkImage img;
    vmaCreateImage(allocator, &(const VkImageCreateInfo &)imgInfo, &allocInfo, &img, &alloc, NULL);
    image = vk::Image(img);
  }

  void destroy() { vmaDestroyImage(allocator, (VkImage)image, alloc); }
};

template <typename VkType>
struct TemplatedResultChecker
{
  TemplatedResultChecker(VkType &val, const char *f, int l) : value(val), file(f), line(l) {}
  ~TemplatedResultChecker()
  {
    if(vkr != vk::Result::eSuccess)
    {
      fprintf(stdout, "%s:%d Vulkan Error: %s\n", file, line, vk::to_string(vkr).c_str());
      fflush(stdout);
      DEBUG_BREAK();
    }
  }

  void operator=(vk::ResultValue<VkType> resultVal)
  {
    vkr = resultVal.result;
    value = resultVal.value;
  }

  const char *file = " ";
  int line = 0;
  VkType &value;
  vk::Result vkr = vk::Result::eSuccess;
};

#define ResultChecker(val) TemplatedResultChecker<decltype(val)>(val, __FILE__, __LINE__)

template <typename ObjType>
vk::Format FormatFromObj();

#define formatof(obj) FormatFromObj<decltype(obj)>()

struct VulkanGraphicsTest : public GraphicsTest
{
  static const TestAPI API = TestAPI::Vulkan;

  VulkanGraphicsTest();
  ~VulkanGraphicsTest();

  bool Init(int argc, char **argv);

  bool Running();
  vk::Image StartUsingBackbuffer(vk::CommandBuffer cmd, vk::AccessFlags nextUse,
                                 vk::ImageLayout layout);
  void FinishUsingBackbuffer(vk::CommandBuffer cmd, vk::AccessFlags prevUse, vk::ImageLayout layout);
  void Submit(int index, int totalSubmits, const std::vector<vk::CommandBuffer> &cmds);
  void Present();

  vk::ShaderModule CompileShaderModule(const std::string &source_text, ShaderLang lang,
                                       ShaderStage stage, const char *entry_point);
  vk::CommandBuffer GetCommandBuffer();

  void resize();

  bool createSwap();
  void destroySwap();
  void acquireImage();

  // requested features
  vk::PhysicalDeviceFeatures features = {};

  // requested extensions
  std::vector<const char *> instExts;
  std::vector<const char *> devExts;

  // core objects
  vk::Instance instance;
  vk::PhysicalDevice phys;
  vk::Device device;
  vk::Queue queue;

  // swapchain stuff
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  vk::SwapchainKHR swap;
  vk::Format swapFormat;
  std::vector<vk::Image> swapImages;
  std::vector<vk::ImageView> swapImageViews;
  uint32_t swapIndex = 0;
  vk::Semaphore renderStartSemaphore, renderEndSemaphore;

  // utilities
  vk::DebugReportCallbackEXT callback;
  vk::CommandPool cmdPool;

  vk::Viewport viewport;
  vk::Rect2D scissor;

  // glfw
  GLFWwindow *win = NULL;
  bool inited = false;

  // internal bookkeeping
  std::set<vk::Fence> fences;

  std::vector<vk::CommandBuffer> freeCommandBuffers;
  std::vector<std::pair<vk::CommandBuffer, vk::Fence>> pendingCommandBuffers;

  // VMA
  VmaAllocator allocator = VK_NULL_HANDLE;
};