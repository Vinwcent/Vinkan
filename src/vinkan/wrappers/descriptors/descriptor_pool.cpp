#include "descriptor_pool.hpp"

// std
#include <stdexcept>

namespace vinkan {

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
  return std::unique_ptr<DescriptorPool>(
      new DescriptorPool(device_, maxSets, poolFlags, poolSizes));
}

DescriptorPool::DescriptorPool(
    VkDevice device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : device_{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(device_, &descriptorPoolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(device_, descriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptorSet(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  if (vkAllocateDescriptorSets(device_, &allocInfo, &descriptor) !=
      VK_SUCCESS) {
    return false;
  }
  return true;
}

void DescriptorPool::freeDescriptors(
    std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(device_, descriptorPool,
                       static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void DescriptorPool::resetPool() {
  vkResetDescriptorPool(device_, descriptorPool, 0);
}

}  // namespace vinkan

