#ifndef VINKAN_PIPELINES_HPP
#define VINKAN_PIPELINES_HPP

#include <map>
#include <vector>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/logging/logger.hpp"
#include "vulkan/vulkan_core.h"

namespace vinkan {

template <EnumType PipelineLayoutT>
class Pipelines {
 public:
  Pipelines(VkDevice device) : device_(device) {}

  template <typename PushConstantT>
  void createLayout(PipelineLayoutT layoutIdentifier,
                    std::vector<VkDescriptorSetLayout> setLayouts,
                    VkShaderStageFlags stageFlags) {
    VkPushConstantRange singlePush{
        .stageFlags = stageFlags,
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

 private:
  VkDevice device_;

  std::map<PipelineLayoutT, VkPipelineLayout> pipelineLayouts_;
};

}  // namespace vinkan
#endif
