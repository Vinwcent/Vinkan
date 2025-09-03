#ifndef VINKAN_PIPELINES_HPP
#define VINKAN_PIPELINES_HPP

#include <map>
#include <vector>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/logging/logger.hpp"
#include "vinkan/pipelines/shader_module_maker.hpp"
#include "vulkan/vulkan_core.h"

namespace vinkan {

template <typename T>
concept ValidShaderInfo =
    std::is_same_v<T, ShaderFileInfo> || std::is_same_v<T, ShaderRawInfo>;

template <EnumType PipelineT, EnumType PipelineLayoutT>
class Pipelines {
 public:
  Pipelines(VkDevice device) : device_(device) {}
  ~Pipelines() {
    for (auto& [identifier, pipeline] : pipelines_) {
      vkDestroyPipeline(device_, pipeline, nullptr);
    }
    for (auto& [identifier, layout] : pipelineLayouts_) {
      vkDestroyPipelineLayout(device_, layout, nullptr);
    }
  }
  Pipelines(const Pipelines&) = delete;
  Pipelines& operator=(const Pipelines&) = delete;

  template <typename PushConstantT>
  void createLayout(PipelineLayoutT layoutIdentifier,
                    std::vector<VkDescriptorSetLayout> setLayouts,
                    VkShaderStageFlags pushConstantStageFlags) {
    VkPushConstantRange singlePush{
        .stageFlags = pushConstantStageFlags,
        .offset = 0,
        .size = sizeof(PushConstantT),
    };
    createLayout(layoutIdentifier, setLayouts, {singlePush});
  }

  void createLayout(PipelineLayoutT layoutIdentifier,
                    std::vector<VkDescriptorSetLayout> setLayouts,
                    std::vector<VkPushConstantRange> pushRanges) {
    VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};
    computePipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutInfo.setLayoutCount =
        static_cast<uint32_t>(setLayouts.size());
    computePipelineLayoutInfo.pSetLayouts = setLayouts.data();
    computePipelineLayoutInfo.pushConstantRangeCount = pushRanges.size();
    computePipelineLayoutInfo.pPushConstantRanges = pushRanges.data();
    computePipelineLayoutInfo.pNext = nullptr;
    computePipelineLayoutInfo.flags = 0;

    if (vkCreatePipelineLayout(device_, &computePipelineLayoutInfo, nullptr,
                               &pipelineLayouts_[layoutIdentifier]) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create pipeline layout");
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Pipeline layout created");
  }

  VkPipelineBindPoint getBindPoint(PipelineT pipelineIdentifier) const {
    assert(pipelineToBindPoints_.contains(pipelineIdentifier));
    return pipelineToBindPoints_.at(pipelineIdentifier);
  }

  template <ValidShaderInfo ShaderInfoT>
  void createComputePipeline(PipelineT pipelineIdentifier,
                             PipelineLayoutT pipelineLayoutIdentifier,
                             ShaderInfoT shaderInfo) {
    assert(pipelineLayouts_.contains(pipelineLayoutIdentifier));
    auto pipelineLayout = pipelineLayouts_[pipelineLayoutIdentifier];

    ShaderModuleMaker moduleMaker(device_);
    auto vkShaderStages = moduleMaker(shaderInfo);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType =
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.stage = vkShaderStages;
    computePipelineCreateInfo.layout = pipelineLayout;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.flags = 0;

    VkPipeline pipeline;
    if (vkCreateComputePipelines(device_, VK_NULL_HANDLE, 1,
                                 &computePipelineCreateInfo, nullptr,
                                 &pipeline) != VK_SUCCESS) {
      throw std::runtime_error("Could not create the compute pipeline");
    }
    pipelines_[pipelineIdentifier] = pipeline;
    pipelineToBindPoints_[pipelineIdentifier] = VK_PIPELINE_BIND_POINT_COMPUTE;

    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Pipeline created");
  }

  void bindCmdBuffer(VkCommandBuffer commandBuffer, PipelineT pipeline) {
    assert(pipelines_.contains(pipeline));
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      pipelines_[pipeline]);
  }

  VkPipelineLayout get(PipelineLayoutT pipelineLayout) {
    assert(pipelineLayouts_.contains(pipelineLayout));
    return pipelineLayouts_[pipelineLayout];
  }

 private:
  VkDevice device_;

  std::map<PipelineT, VkPipelineBindPoint> pipelineToBindPoints_;
  std::map<PipelineT, VkPipeline> pipelines_;
  std::map<PipelineLayoutT, VkPipelineLayout> pipelineLayouts_;
};

}  // namespace vinkan
#endif
