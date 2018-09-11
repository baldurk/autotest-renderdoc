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

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#include "vk_common.h"

#include <vulkan/vk_mem_alloc.h>

static VkBool32 VKAPI_PTR vulkanCallback(VkDebugReportFlagsEXT flags,
                                         VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                         size_t location, int32_t messageCode,
                                         const char *pLayerPrefix, const char *pMessage,
                                         void *pUserData)
{
  TEST_WARN("Vulkan %s message: [%s] %s", vk::to_string((vk::DebugReportFlagBitsEXT)flags).c_str(),
            pLayerPrefix, pMessage);

  return false;
}

static void glfwCallback(int err, const char *str)
{
  TEST_ERROR("GLFW Error %i: %s", err, str);
}

static void glfwKeypress(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if(key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, 1);
}

static void glfwResize(GLFWwindow *window, int width, int height)
{
  if(width == 0 || height == 0)
    return;

  VulkanGraphicsTest *test = (VulkanGraphicsTest *)glfwGetWindowUserPointer(window);
  test->resize();
}

VulkanGraphicsTest::VulkanGraphicsTest()
{
  features.depthClamp = true;
}

bool VulkanGraphicsTest::Init(int argc, char **argv)
{
  // parse parameters here to override parameters
  GraphicsTest::Init(argc, argv);

  if(volkInitialize() != VK_SUCCESS)
  {
    TEST_ERROR("Couldn't init vulkan");
    return false;
  }

  if(!SpvCompilationSupported())
  {
    TEST_ERROR("glslc must be in PATH to run vulkan tests");
    return false;
  }

  if(!glfwInit())
  {
    TEST_ERROR("Failed to init GLFW");
    return false;
  }

  inited = true;

  if(!glfwVulkanSupported())
  {
    TEST_ERROR("Vulkan is not supported");
    return false;
  }

  glfwSetErrorCallback(&glfwCallback);

  glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
  glfwWindowHint(GLFW_RESIZABLE, 0);

  uint32_t count = 0;
  const char *const *extensions = glfwGetRequiredInstanceExtensions(&count);

  if(!extensions)
  {
    TEST_ERROR("GLFW cannot support vulkan rendering");
    return false;
  }

  vk::Result vkr = vk::Result::eSuccess;

  vk::ApplicationInfo app("RenderDoc autotesting", VK_MAKE_VERSION(1, 0, 0),
                          "RenderDoc autotesting", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

  for(uint32_t i = 0; i < count; i++)
    instExts.push_back(extensions[i]);

  instExts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

  std::vector<const char *> layers;

  std::vector<vk::LayerProperties> supportedLayers;
  ResultChecker(supportedLayers) = vk::enumerateInstanceLayerProperties();

  for(const vk::LayerProperties &layer : supportedLayers)
  {
    if(!strcmp(layer.layerName, "VK_LAYER_LUNARG_standard_validation"))
    {
      layers.push_back(layer.layerName);
      break;
    }
  }

  std::vector<vk::ExtensionProperties> supportedExts;
  ResultChecker(supportedExts) = vk::enumerateInstanceExtensionProperties();

  std::tie(vkr, instance) = vk::createInstance(vk::InstanceCreateInfo(
      {}, &app, (uint32_t)layers.size(), layers.data(), (uint32_t)instExts.size(), instExts.data()));

  if(vkr != vk::Result::eSuccess)
  {
    TEST_ERROR("Error creating instance: %s", vk::to_string(vkr).c_str());
    return false;
  }

  volkLoadInstance((VkInstance)instance);

  ResultChecker(callback) =
      instance.createDebugReportCallbackEXT(vk::DebugReportCallbackCreateInfoEXT(
          vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
          &vulkanCallback, NULL));

  std::vector<vk::PhysicalDevice> physDevices;

  std::tie(vkr, physDevices) = instance.enumeratePhysicalDevices();

  if(vkr != vk::Result::eSuccess || physDevices.empty())
  {
    TEST_ERROR("Error creating instance: %s", vk::to_string(vkr).c_str());
    return false;
  }

  phys = physDevices[0];

  std::vector<vk::QueueFamilyProperties> queueProps = phys.getQueueFamilyProperties();

  uint32_t queueFamilyIdx = 0;
  for(queueFamilyIdx = 0; queueFamilyIdx < queueProps.size(); queueFamilyIdx++)
  {
    vk::QueueFlags flags = queueProps[queueFamilyIdx].queueFlags;
    if((flags & vk::QueueFlagBits::eCompute) && (flags & vk::QueueFlagBits::eGraphics))
    {
      break;
    }
  }

  if(glfwGetPhysicalDevicePresentationSupport((VkInstance)instance, (VkPhysicalDevice)phys,
                                              queueFamilyIdx))
  {
    // Queue family supports image presentation
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  win = glfwCreateWindow(screenWidth, screenHeight, "Autotesting", NULL, NULL);

  glfwSetKeyCallback(win, &glfwKeypress);

  glfwSetWindowUserPointer(win, this);
  glfwSetWindowSizeCallback(win, &glfwResize);

  if(!win)
  {
    TEST_ERROR("Error creating glfw window");
    return false;
  }

  vkr = (vk::Result)glfwCreateWindowSurface((VkInstance)instance, win, NULL, &surface);

  if(vkr != vk::Result::eSuccess)
  {
    TEST_ERROR("Error creating glfw surface: %s", vk::to_string(vkr).c_str());
    return false;
  };

  devExts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  float prio = 1.0f;
  vk::DeviceQueueCreateInfo queueInfo({}, 0, 1, &prio);

  vk::PhysicalDeviceFeatures supported = phys.getFeatures();

  const vk::Bool32 *enabledBegin = (vk::Bool32 *)&features;
  const vk::Bool32 *enabledEnd = enabledBegin + sizeof(features);

  const vk::Bool32 *supportedBegin = (vk::Bool32 *)&features;
  const vk::Bool32 *supportedEnd = supportedBegin + sizeof(features);

  for(; enabledBegin != enabledEnd; ++enabledBegin, ++supportedBegin)
  {
    if(*enabledBegin && !*supportedBegin)
    {
      TEST_ERROR("Feature enabled that isn't supported");
      return false;
    }
  }

  std::tie(vkr, device) = phys.createDevice(
      vk::DeviceCreateInfo({}, 1, &queueInfo, (uint32_t)layers.size(), layers.data(),
                           (uint32_t)devExts.size(), devExts.data(), &features));

  if(vkr != vk::Result::eSuccess)
  {
    TEST_ERROR("Error creating device: %s", vk::to_string(vkr).c_str());
    return false;
  }

  queue = device.getQueue(0, 0);

  ResultChecker(renderStartSemaphore) = device.createSemaphore({});
  ResultChecker(renderEndSemaphore) = device.createSemaphore({});

  ResultChecker(cmdPool) = device.createCommandPool(
      vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));

  createSwap();

  VmaVulkanFunctions funcs = {
      vkGetPhysicalDeviceProperties,
      vkGetPhysicalDeviceMemoryProperties,
      vkAllocateMemory,
      vkFreeMemory,
      vkMapMemory,
      vkUnmapMemory,
      vkFlushMappedMemoryRanges,
      vkInvalidateMappedMemoryRanges,
      vkBindBufferMemory,
      vkBindImageMemory,
      vkGetBufferMemoryRequirements,
      vkGetImageMemoryRequirements,
      vkCreateBuffer,
      vkDestroyBuffer,
      vkCreateImage,
      vkDestroyImage,
      vkGetBufferMemoryRequirements2KHR,
      vkGetImageMemoryRequirements2KHR,
  };

  VmaAllocatorCreateInfo allocInfo = {};
  allocInfo.physicalDevice = (VkPhysicalDevice)phys;
  allocInfo.device = (VkDevice)device;
  allocInfo.frameInUseCount = uint32_t(swapImages.size() - 1);
  allocInfo.pVulkanFunctions = &funcs;

  vmaCreateAllocator(&allocInfo, &allocator);

  return true;
}

VulkanGraphicsTest::~VulkanGraphicsTest()
{
  vmaDestroyAllocator(allocator);

  if(device)
  {
    device.waitIdle();

    for(vk::Fence fence : fences)
      device.destroyFence(fence);

    device.destroyCommandPool(cmdPool);
    device.destroySemaphore(renderStartSemaphore);
    device.destroySemaphore(renderEndSemaphore);

    destroySwap();

    device.destroy();
  }

  if(callback)
    instance.destroyDebugReportCallbackEXT(callback);
  if(surface)
    vkDestroySurfaceKHR((VkInstance)instance, surface, NULL);

  if(instance)
    instance.destroy();

  if(win)
    glfwDestroyWindow(win);

  if(inited)
    glfwTerminate();
}

bool VulkanGraphicsTest::Running()
{
  if(glfwWindowShouldClose(win))
    return false;

  return true;
}

vk::Image VulkanGraphicsTest::StartUsingBackbuffer(vk::CommandBuffer cmd, vk::AccessFlags nextUse,
                                                   vk::ImageLayout layout)
{
  vk::Image img = swapImages[swapIndex];

  cmd.pipelineBarrier(
      vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {},
      {vk::ImageMemoryBarrier({}, nextUse, vk::ImageLayout::eUndefined, layout, 0, 0, img)});

  return img;
}

void VulkanGraphicsTest::FinishUsingBackbuffer(vk::CommandBuffer cmd, vk::AccessFlags prevUse,
                                               vk::ImageLayout layout)
{
  vk::Image img = swapImages[swapIndex];

  cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                      vk::PipelineStageFlagBits::eAllCommands, {}, {}, {},
                      {vk::ImageMemoryBarrier(prevUse, vk::AccessFlagBits::eMemoryRead, layout,
                                              vk::ImageLayout::ePresentSrcKHR, 0, 0, img)});
}

void VulkanGraphicsTest::Submit(int index, int totalSubmits,
                                const std::vector<vk::CommandBuffer> &cmds)
{
  vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
  vk::SubmitInfo submit(0, NULL, &waitStage, (uint32_t)cmds.size(), cmds.data(), 0, NULL);
  if(index == 0)
    submit.setPWaitSemaphores(&renderStartSemaphore).setWaitSemaphoreCount(1);

  if(index == totalSubmits - 1)
    submit.setPSignalSemaphores(&renderEndSemaphore).setSignalSemaphoreCount(1);

  vk::Fence fence;
  ResultChecker(fence) = device.createFence({});

  fences.insert(fence);

  for(const vk::CommandBuffer &cmd : cmds)
    pendingCommandBuffers.push_back(std::make_pair(cmd, fence));

  queue.submit({submit}, fence);
}

void VulkanGraphicsTest::Present()
{
  vk::Result vkr = queue.presentKHR(vk::PresentInfoKHR(1, &renderEndSemaphore, 1, &swap, &swapIndex));

  if(vkr == vk::Result::eSuboptimalKHR || vkr == vk::Result::eErrorOutOfDateKHR)
    resize();

  queue.waitIdle();
  glfwPollEvents();

  std::set<vk::Fence> doneFences;

  for(auto it = pendingCommandBuffers.begin(); it != pendingCommandBuffers.end();)
  {
    if(device.getFenceStatus(it->second) == vk::Result::eSuccess)
    {
      freeCommandBuffers.push_back(it->first);
      doneFences.insert(it->second);
      it = pendingCommandBuffers.erase(it);
    }
    else
    {
      ++it;
    }
  }

  for(auto it = doneFences.begin(); it != doneFences.end(); ++it)
  {
    device.destroyFence(*it);
    fences.erase(*it);
  }

  acquireImage();
}

vk::ShaderModule VulkanGraphicsTest::CompileShaderModule(const std::string &source_text,
                                                         ShaderLang lang, ShaderStage stage,
                                                         const char *entry_point)
{
  vk::ShaderModule ret;

  std::vector<uint32_t> spirv = ::CompileShaderToSpv(source_text, lang, stage, entry_point);

  if(spirv.empty())
    return ret;

  ResultChecker(ret) = device.createShaderModule(
      vk::ShaderModuleCreateInfo({}, spirv.size() * sizeof(uint32_t), spirv.data()));

  return ret;
}

vk::CommandBuffer VulkanGraphicsTest::GetCommandBuffer()
{
  if(freeCommandBuffers.empty())
  {
    ResultChecker(freeCommandBuffers) = device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(cmdPool, vk::CommandBufferLevel::ePrimary, 4));
  }

  vk::CommandBuffer ret = freeCommandBuffers.back();
  freeCommandBuffers.pop_back();

  return ret;
}

void VulkanGraphicsTest::resize()
{
  destroySwap();

  createSwap();
}

bool VulkanGraphicsTest::createSwap()
{
  vk::Result vkr = vk::Result::eSuccess;

  vk::SurfaceFormatKHR format = {};

  std::vector<vk::SurfaceFormatKHR> formats;
  std::tie(vkr, formats) = phys.getSurfaceFormatsKHR(vk::SurfaceKHR(surface));

  vk::Bool32 support;
  std::tie(vkr, support) = phys.getSurfaceSupportKHR(0, vk::SurfaceKHR(surface));
  TEST_ASSERT(support, "Presentation is not supported on surface");

  if(vkr != vk::Result::eSuccess || formats.empty())
  {
    TEST_ERROR("Error getting surface formats: %s", vk::to_string(vkr).c_str());
    return false;
  }

  format = formats[0];

  for(const vk::SurfaceFormatKHR &f : formats)
  {
    if(f.format == vk::Format::eB8G8R8A8Unorm && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
    {
      format = f;
      break;
    }
  }

  if(format.format == vk::Format::eUndefined)
  {
    format.format = vk::Format::eB8G8R8A8Unorm;
    format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
  }

  swapFormat = format.format;

  std::vector<vk::PresentModeKHR> modes;
  std::tie(vkr, modes) = phys.getSurfacePresentModesKHR(vk::SurfaceKHR(surface));

  vk::PresentModeKHR mode = vk::PresentModeKHR::eImmediate;

  if(std::find(modes.begin(), modes.end(), mode) == modes.end())
    mode = vk::PresentModeKHR::eFifo;

  uint32_t width, height;
  glfwGetWindowSize(win, (int *)&width, (int *)&height);

  vk::SurfaceCapabilitiesKHR capabilities;
  std::tie(vkr, capabilities) = phys.getSurfaceCapabilitiesKHR(vk::SurfaceKHR(surface));

  width = std::min(width, capabilities.maxImageExtent.width);
  width = std::max(width, capabilities.minImageExtent.width);

  height = std::min(height, capabilities.maxImageExtent.height);
  height = std::max(height, capabilities.minImageExtent.height);

  viewport = vk::Viewport(0, 0, (float)width, (float)height, 0.0f, 1.0f);
  scissor = vk::Rect2D({0, 0}, {width, height});

  uint32_t queueFamilyIndex = 0;
  std::tie(vkr, swap) = device.createSwapchainKHR(vk::SwapchainCreateInfoKHR(
      {}, vk::SurfaceKHR(surface), 2, format.format, format.colorSpace, vk::Extent2D(width, height),
      1, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment,
      vk::SharingMode::eExclusive, 1, &queueFamilyIndex, vk::SurfaceTransformFlagBitsKHR::eIdentity,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, mode, 0, swap));

  if(vkr != vk::Result::eSuccess)
  {
    TEST_ERROR("Error creating swapchain: %s", vk::to_string(vkr).c_str());
    return false;
  }

  std::tie(vkr, swapImages) = device.getSwapchainImagesKHR(swap);

  if(vkr != vk::Result::eSuccess)
  {
    TEST_ERROR("Error getting swapchain images: %s", vk::to_string(vkr).c_str());
    return false;
  }

  swapImageViews.resize(swapImages.size());
  for(size_t i = 0; i < swapImages.size(); i++)
  {
    std::tie(vkr, swapImageViews[i]) = device.createImageView(vk::ImageViewCreateInfo(
        {}, swapImages[i], vk::ImageViewType::e2D, format.format, {}, vk::ImageSubresourceRange()));

    if(vkr != vk::Result::eSuccess)
    {
      TEST_ERROR("Error creating swapchain image view %u: %s", (uint32_t)i,
                 vk::to_string(vkr).c_str());
      return false;
    }
  }

  acquireImage();

  return true;
}

void VulkanGraphicsTest::destroySwap()
{
  device.waitIdle();

  for(size_t i = 0; i < swapImages.size(); i++)
    device.destroyImageView(swapImageViews[i]);

  device.destroySwapchainKHR(swap);
}

void VulkanGraphicsTest::acquireImage()
{
  vk::Result vkr;

  std::tie(vkr, swapIndex) =
      device.acquireNextImageKHR(swap, UINT64_MAX, renderStartSemaphore, vk::Fence());

  if(vkr == vk::Result::eSuboptimalKHR || vkr == vk::Result::eErrorOutOfDateKHR)
  {
    resize();
    return;
  }
}

PipelineCreator::PipelineCreator()
{
  // defaults
  // 1 viewport/scissor, dynamic
  dyns = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vp.setScissorCount(1).setViewportCount(1);

  ia.setTopology(vk::PrimitiveTopology::eTriangleList);

  rs.setCullMode(vk::CullModeFlagBits::eNone)
      .setDepthClampEnable(true)
      .setFrontFace(vk::FrontFace::eClockwise)
      .setLineWidth(1.0f)
      .setPolygonMode(vk::PolygonMode::eFill);

  msaa.setRasterizationSamples(vk::SampleCountFlagBits::e1);

  cb.setBlendConstants(std::array<float, 4>({1.0f, 1.0f, 1.0f, 1.0f})).setLogicOp(vk::LogicOp::eNoOp);

  blends.push_back(vk::PipelineColorBlendAttachmentState(
      0, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eRGBA));
}

vk::GraphicsPipelineCreateInfo PipelineCreator::bake()
{
  dynamic.setPDynamicStates(dyns.data()).setDynamicStateCount((uint32_t)dyns.size());

  cb.setPAttachments(blends.data()).setAttachmentCount((uint32_t)blends.size());

  vi.setPVertexBindingDescriptions(binds.data())
      .setVertexBindingDescriptionCount((uint32_t)binds.size())
      .setPVertexAttributeDescriptions(attrs.data())
      .setVertexAttributeDescriptionCount((uint32_t)attrs.size());

  return vk::GraphicsPipelineCreateInfo()
      .setLayout(layout)
      .setRenderPass(renderPass)
      .setPStages(stages.data())
      .setStageCount((uint32_t)stages.size())
      .setPDynamicState(&dynamic)
      .setPVertexInputState(&vi)
      .setPInputAssemblyState(&ia)
      .setPViewportState(&vp)
      .setPRasterizationState(&rs)
      .setPDepthStencilState(&ds)
      .setPColorBlendState(&cb)
      .setPMultisampleState(&msaa);
}

template <>
vk::Format FormatFromObj<Vec4f>()
{
  return vk::Format::eR32G32B32A32Sfloat;
}
template <>
vk::Format FormatFromObj<Vec3f>()
{
  return vk::Format::eR32G32B32Sfloat;
}
template <>
vk::Format FormatFromObj<Vec2f>()
{
  return vk::Format::eR32G32Sfloat;
}
template <>
vk::Format FormatFromObj<float>()
{
  return vk::Format::eR32Sfloat;
}