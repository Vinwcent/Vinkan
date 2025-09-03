#ifndef VINKAN_PIPELINE_INFO_HPP
#define VINKAN_PIPELINE_INFO_HPP
#include "vinkan/generics/concepts.hpp"
namespace vinkan {

template <EnumType PipelineLayoutT, ValidShaderInfo ShaderInfoT>
struct ComputePipelineInfo {
  PipelineLayoutT layoutIdentifier;
  ShaderInfoT shaderInfo;
};

template <EnumType PipelineLayoutT, ValidShaderInfo ShaderInfoT>
struct GraphicsPipelineInfo {
  // Resources info
  PipelineLayoutT layoutIdentifier;

  // Input info
  ShaderInfoT vertexShaderInfo;
  ShaderInfoT fragmentShaderInfo;
  VkPipelineVertexInputStateCreateInfo vertexInputState;

  // RenderPass info
  VkRenderPass renderPass;
  uint32_t subpass;

  // Configuration info
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
};

}  // namespace vinkan
#endif
