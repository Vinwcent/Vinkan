#ifndef VINKAN_HELLO_TRIANGLE_HPP
#define VINKAN_HELLO_TRIANGLE_HPP

#include <vulkan/vulkan.h>

#include <vector>
#include <vinkan/vinkan.hpp>

const std::vector<const char*> MyAppValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Queue
enum class MyAppQueue { GRAPHICS_AND_PRESENT_QUEUE };

// Pipeline
enum class MyAppPipelineLayout { GRAPHICS_PIP_LAYOUT };
enum class MyAppPipeline { GRAPHICS_PIPELINE };

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

namespace vinkan {

inline void createGfxPipeline(
    vinkan::Pipelines<MyAppPipeline, MyAppPipelineLayout>& pipelines,
    const VkExtent2D& imageExtent,
    const vinkan::ShaderFileInfo& vertexShaderFileInfo,
    const vinkan::ShaderFileInfo& fragmentShaderFileInfo,
    VkRenderPass renderPass,
    const VkVertexInputBindingDescription& bindingDescription,
    const std::vector<VkVertexInputAttributeDescription>&
        attributeDescriptions) {
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data()};

  static VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
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

  static VkPipelineRasterizationStateCreateInfo rasterizationInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f};

  static VkPipelineMultisampleStateCreateInfo multisampleInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE};

  static VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  static VkPipelineColorBlendStateCreateInfo colorBlendInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  static VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE};

  static VkPipelineDynamicStateCreateInfo dynamicStateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 0,
      .pDynamicStates = nullptr};

  auto gfxPipelineInfo =
      vinkan::GraphicsPipelineInfo<MyAppPipelineLayout, vinkan::ShaderFileInfo>{
          .layoutIdentifier = MyAppPipelineLayout::GRAPHICS_PIP_LAYOUT,
          .vertexShaderInfo = vertexShaderFileInfo,
          .fragmentShaderInfo = fragmentShaderFileInfo,
          .vertexInputState = vertexInputInfo,
          .renderPass = renderPass,
          .subpass = 0,
          .inputAssemblyInfo = inputAssemblyInfo,
          .viewportInfo = viewportInfo,
          .rasterizationInfo = rasterizationInfo,
          .multisampleInfo = multisampleInfo,
          .colorBlendInfo = colorBlendInfo,
          .depthStencilInfo = depthStencilInfo,
          .dynamicStateInfo = dynamicStateInfo};

  pipelines.createGraphicsPipeline(MyAppPipeline::GRAPHICS_PIPELINE,
                                   gfxPipelineInfo);
}

}  // namespace vinkan

#endif
