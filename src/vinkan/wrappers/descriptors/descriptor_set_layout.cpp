#include "descriptor_set_layout.hpp"

// std
#include <cassert>
#include <stdexcept>
#include <vector>

namespace vinkan {

///////////////
//  BUILDER  //
///////////////

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    DescriptorSetLayoutBinding descriptorSetLayoutBinding) {
  assert(!vkBindings_.contains(descriptorSetLayoutBinding.bindingIndex) &&
         "Descriptor set layout already uses this binding");
  VkDescriptorSetLayoutBinding vkBinding{};
  vkBinding.binding = descriptorSetLayoutBinding.bindingIndex;
  vkBinding.descriptorType = descriptorSetLayoutBinding.descriptorType;
  vkBinding.stageFlags = descriptorSetLayoutBinding.shaderStageFlags;
  vkBinding.descriptorCount = descriptorSetLayoutBinding.count;
  vkBindings_.emplace(descriptorSetLayoutBinding.bindingIndex, vkBinding);
  return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build()
    const {
  auto setLayout = std::unique_ptr<DescriptorSetLayout>(
      new DescriptorSetLayout(device_, vkBindings_));
  return setLayout;
}

///////////////
//  WRAPPER  //
///////////////

DescriptorSetLayout::DescriptorSetLayout(
    VkDevice device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : device_(device), bindings_(bindings) {
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
  for (const auto &[bindingIndex, binding] : bindings) {
    layoutBindings.push_back(binding);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount =
      static_cast<uint32_t>(layoutBindings.size());
  descriptorSetLayoutInfo.pBindings = layoutBindings.data();

  if (vkCreateDescriptorSetLayout(device_, &descriptorSetLayoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Could not create the descriptor set layout");
  }
}

DescriptorSetLayout::~DescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(device_, descriptorSetLayout, nullptr);
}

VkDescriptorSetLayoutBinding DescriptorSetLayout::getLayoutBinding(
    uint32_t bindingIndex) const {
  assert(bindings_.contains(bindingIndex) && "No binding with this index");
  return bindings_.at(bindingIndex);
}

}  // namespace vinkan

