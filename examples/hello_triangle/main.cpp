#include <GLFW/glfw3.h>

#include <vinkan/pipelines/shader_module_maker.hpp>
#include <vinkan/vinkan.hpp>

#include "extensions.hpp"

const std::vector<const char *> MyAppValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Queue
enum class MyAppQueue { GRAPHICS_AND_PRESENT_QUEUE };

// Pipeline
enum class MyAppPipeline { GRAPHICS_PIPELINE };
enum class MyAppPipelineLayout { GRAPHICS_PIP_LAYOUT };

// RenderPass
enum class MyAppAttachment { SWAPCHAIN_ATTACHMENT };

// Command buffers
enum class MyAppCommandBuffer { GRAPHICS_CMD_1, GRAPHICS_CMD_2, TRANSFER_CMD };
enum class MyAppCommandPool { GRAPHICS_POOL };
enum class MyAppFence { GRAPHICS_FENCE };
enum class MyAppSemaphore { IMG_AVAILABLE, DRAW_FINISH };

// Push constants
struct MyAppPC {
  uint32_t value;
};

// Vertex structure
struct Vertex {
  float position[3];

  static VkVertexInputBindingDescription getBinding() {
    return VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributes() {
    return {VkVertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position)}};
  }
};

int main() {
  // Create the window (with glfw)
  glfwInit();
  // No opengl since we use Vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  int width, height, xpos, ypos;
  glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
  VkExtent2D windowExtent{.width = 500, .height = 500};
  auto window = glfwCreateWindow(windowExtent.width, windowExtent.height,
                                 "Example triangle app", nullptr, nullptr);

  // Create the instance
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  // Get the glfw extensions
  std::vector<const char *> extraVkExtensions(
      glfwExtensions, glfwExtensions + glfwExtensionCount);
  extraVkExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  vinkan::InstanceInfo instanceInfo{
      .appName = "Example triangle app",
      .appVersion = vinkan::SemanticVersion{.major = 0, .minor = 0, .patch = 1},
      .engineName = "Example triangle engine",
      .engineVersion =
          vinkan::SemanticVersion{.major = 0, .minor = 0, .patch = 1},
      .apiVersion = VK_API_VERSION_1_2,
      .validationLayers = MyAppValidationLayers,
      .includePortabilityExtensions = NEED_PORTABILITY_EXTENSIONS,
      .extraVkExtensions = extraVkExtensions};
  vinkan::Instance instance(instanceInfo);

  // Create the surface
  vinkan::GlfwVkSurface surface(window, instance.getHandle());

  // Create the physical device
  vinkan::PhysicalDeviceInfo physicalDeviceInfo{
      .requestedQueueFlags = {VK_QUEUE_GRAPHICS_BIT},
      .surfaceSupportRequested = surface.getHandle(),
      .extensions = DEVICE_EXTENSIONS};
  vinkan::PhysicalDevice physicalDevice(physicalDeviceInfo,
                                        instance.getHandle());

  // Create the device (with one graphics queue on one queue family)
  vinkan::Device<MyAppQueue>::Builder deviceBuilder(physicalDevice.getHandle(),
                                                    physicalDevice.getQueues());
  deviceBuilder.addExtensions(DEVICE_EXTENSIONS);
  vinkan::QueueFamilyRequest<MyAppQueue> queueRequest{
      .queueFamilyIdentifier = MyAppQueue::GRAPHICS_AND_PRESENT_QUEUE,
      .flagsRequested = VK_QUEUE_GRAPHICS_BIT,
      .surfacePresentationSupport = surface.getHandle(),
      .nQueues = 1,
      .queuePriorities = {1.0}};
  bool success = false;
  bool mustBeInNewQueueFamily = true;
  deviceBuilder.addQueue(queueRequest, mustBeInNewQueueFamily, success);
  assert(success);
  auto device = deviceBuilder.build();

  // Swapchain
  auto surfaceDetails = physicalDevice.getSurfaceSupportDetails();

  std::vector<VkPresentModeKHR> wantedPresentModes = {
      VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};
  auto presentMode = surfaceDetails.findBestPresentMode(wantedPresentModes);
  assert(presentMode.has_value());
  auto imageExtent = surfaceDetails.selectExtent(windowExtent);
  std::vector<VkSurfaceFormatKHR> wantedFormats = {
      {.format = VK_FORMAT_B8G8R8A8_SRGB,
       .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR}};
  auto surfaceFormat = surfaceDetails.findBestFormat(wantedFormats);
  assert(surfaceFormat.has_value());
  auto imageCount = std::min(2u, surfaceDetails.capabilities.maxImageCount);

  vinkan::SwapchainInfo swapchainInfo{
      .device = device->getHandle(),
      .surface = surface.getHandle(),
      .imageExtent = imageExtent,
      .surfaceFormat = surfaceFormat.value(),
      .presentMode = presentMode.value(),
      .imageCount = imageCount,
      .sharingMode = vinkan::SharingMode{.value = VK_SHARING_MODE_EXCLUSIVE},
      .surfaceSupportDetails = surfaceDetails};
  vinkan::Swapchain swapchain(swapchainInfo);

  // Renderpass
  vinkan::RenderPass<MyAppAttachment>::Builder renderPassBuilder;
  renderPassBuilder.addAttachment(
      MyAppAttachment::SWAPCHAIN_ATTACHMENT,
      VkAttachmentDescription{
          .format = surfaceFormat.value().format,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      },
      VK_IMAGE_LAYOUT_GENERAL);

  vinkan::SubpassInfo<MyAppAttachment> subpassInfo{
      .colorAttachments = {MyAppAttachment::SWAPCHAIN_ATTACHMENT},
      .inputAttachments = {},
      .preserveAttachments = {},
      .depthStencilAttachment = {},
      .dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      .dstFlags = VK_ACCESS_SHADER_READ_BIT};
  renderPassBuilder.addSubpass(subpassInfo);
  auto renderPass = renderPassBuilder.build(device->getHandle());

  // Initialize the pipelines
  vinkan::Pipelines<MyAppPipeline, MyAppPipelineLayout> pipelines_(
      device->getHandle());

  // Create a pipeline layout
  pipelines_.createLayout<MyAppPC>(MyAppPipelineLayout::GRAPHICS_PIP_LAYOUT, {},
                                   VK_SHADER_STAGE_FRAGMENT_BIT);

  // Create a pipeline with this layout
  vinkan::ShaderFileInfo vertexShaderFileInfo{
      .shaderFilepath = std::string(COMPILED_SHADERS_DIR) + "/vertex.spv",
      .shaderStage = VK_SHADER_STAGE_VERTEX_BIT};
  vinkan::ShaderFileInfo fragmentShaderFileInfo{
      .shaderFilepath = std::string(COMPILED_SHADERS_DIR) + "/fragment.spv",
      .shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT};

  // Vertex input state
  auto bindingDescription = Vertex::getBinding();
  auto attributeDescriptions = Vertex::getAttributes();
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data()};

  // Configuration states
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  VkViewport viewport{.x = 0.0f,
                      .y = 0.0f,
                      .width = static_cast<float>(imageExtent.width),
                      .height = static_cast<float>(imageExtent.height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {0, 0}, .extent = imageExtent};

  VkPipelineViewportStateCreateInfo viewportInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor};

  VkPipelineRasterizationStateCreateInfo rasterizationInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f};

  VkPipelineMultisampleStateCreateInfo multisampleInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE};

  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  VkPipelineColorBlendStateCreateInfo colorBlendInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE};

  VkPipelineDynamicStateCreateInfo dynamicStateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 0,
      .pDynamicStates = nullptr};

  vinkan::GraphicsPipelineInfo<MyAppPipelineLayout, vinkan::ShaderFileInfo>
      gfxPipelineInfo{
          .layoutIdentifier = MyAppPipelineLayout::GRAPHICS_PIP_LAYOUT,
          .vertexShaderInfo = vertexShaderFileInfo,
          .fragmentShaderInfo = fragmentShaderFileInfo,
          .vertexInputState = vertexInputInfo,
          .renderPass = renderPass->getHandle(),
          .subpass = 0,
          .inputAssemblyInfo = inputAssemblyInfo,
          .viewportInfo = viewportInfo,
          .rasterizationInfo = rasterizationInfo,
          .multisampleInfo = multisampleInfo,
          .colorBlendInfo = colorBlendInfo,
          .depthStencilInfo = depthStencilInfo,
          .dynamicStateInfo = dynamicStateInfo};

  pipelines_.createGraphicsPipeline(MyAppPipeline::GRAPHICS_PIPELINE,
                                    gfxPipelineInfo);

  vinkan::RenderStage::Builder<MyAppAttachment> builder(
      *renderPass, device->getHandle(), imageCount);
  builder.defineAttachment(MyAppAttachment::SWAPCHAIN_ATTACHMENT,
                           swapchain.getImageViews());
  std::unique_ptr<vinkan::RenderStage> renderStage =
      builder.build(swapchain.getSwapchainInfo().imageExtent);

  vinkan::SyncMechanisms<MyAppFence, MyAppSemaphore> syncMechanisms(
      device->getHandle());
  syncMechanisms.createFence(MyAppFence::GRAPHICS_FENCE, true);
  syncMechanisms.createSemaphore(MyAppSemaphore::IMG_AVAILABLE);
  syncMechanisms.createSemaphore(MyAppSemaphore::DRAW_FINISH);
  VkFence gfxFence = syncMechanisms.getFence(MyAppFence::GRAPHICS_FENCE);
  VkSemaphore imgAvailableS =
      syncMechanisms.getSemaphore(MyAppSemaphore::IMG_AVAILABLE);
  VkSemaphore drawFinish =
      syncMechanisms.getSemaphore(MyAppSemaphore::DRAW_FINISH);

  // Initialize commands
  vinkan::CommandCoordinator<MyAppCommandBuffer, MyAppCommandPool> coordinator(
      device->getHandle());
  // Create command pool
  coordinator.createCommandPool(
      MyAppCommandPool::GRAPHICS_POOL,
      device->getQueueFamilyIndex(MyAppQueue::GRAPHICS_AND_PRESENT_QUEUE),
      false);
  // Create 2 commands
  coordinator.createLongLivedCommand(MyAppCommandBuffer::GRAPHICS_CMD_1,
                                     MyAppCommandPool::GRAPHICS_POOL);
  coordinator.createLongLivedCommand(MyAppCommandBuffer::GRAPHICS_CMD_2,
                                     MyAppCommandPool::GRAPHICS_POOL);
  coordinator.createLongLivedCommand(MyAppCommandBuffer::TRANSFER_CMD,
                                     MyAppCommandPool::GRAPHICS_POOL);

  // Create triangle model
  std::vector<Vertex> triangleVertices = {
      {{0.0f, -0.5f, 0.0f}},  // Bottom
      {{0.5f, 0.5f, 0.0f}},   // Top right
      {{-0.5f, 0.5f, 0.0f}}   // Top left
  };

  vinkan::ModelData<Vertex> triangleData{triangleVertices, {}};
  vinkan::Model<Vertex> triangle(
      device->getHandle(), physicalDevice.getMemoryProperties(), triangleData);

  // Setup transfer for triangle
  coordinator.resetCommandBuffer(MyAppCommandBuffer::TRANSFER_CMD);
  VkCommandBuffer transferCmd =
      coordinator.get(MyAppCommandBuffer::TRANSFER_CMD);
  VkQueue transferQueue =
      device->getQueue(MyAppQueue::GRAPHICS_AND_PRESENT_QUEUE, 0);
  triangle.transferModelToDevice(transferCmd, triangleData, transferQueue);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Wait for previous frame
    vkWaitForFences(device->getHandle(), 1, &gfxFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &gfxFence);

    // Acquire next image
    auto imageNumberOpt =
        swapchain.acquireNextImageIndex(imgAvailableS, VK_NULL_HANDLE);
    if (!imageNumberOpt.has_value()) {
      throw std::runtime_error("No support for resizing");
    }
    auto imageNumber = imageNumberOpt.value();

    // Select command buffer (alternate between the two)
    MyAppCommandBuffer currentCmd = (imageNumber % 2 == 0)
                                        ? MyAppCommandBuffer::GRAPHICS_CMD_1
                                        : MyAppCommandBuffer::GRAPHICS_CMD_2;

    // Record command buffer
    coordinator.resetCommandBuffer(currentCmd);
    auto commandBuffer = coordinator.beginCommandBuffer(currentCmd);

    // Begin render pass
    renderStage->beginRenderPass(commandBuffer, imageNumber);

    // Bind pipeline
    pipelines_.bindCmdBuffer(commandBuffer, MyAppPipeline::GRAPHICS_PIPELINE);
    // Draw triangle
    triangle.draw(commandBuffer);

    // End render pass
    vkCmdEndRenderPass(commandBuffer);
    coordinator.endCommandBuffer(commandBuffer);

    // Submit
    VkQueue queue = device->getQueue(MyAppQueue::GRAPHICS_AND_PRESENT_QUEUE, 0);
    vinkan::SubmitCommandBufferInfo submitInfo{
        .waitSemaphores = {imgAvailableS},
        .waitDstStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .signalSemaphores = {drawFinish},
        .signalFence = gfxFence,
        .queue = device->getQueue(MyAppQueue::GRAPHICS_AND_PRESENT_QUEUE, 0)};
    coordinator.submitCommandBuffer(commandBuffer, submitInfo);

    // Present
    swapchain.present(imageNumber, queue, drawFinish);
  }

  vkDeviceWaitIdle(device->getHandle());
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
