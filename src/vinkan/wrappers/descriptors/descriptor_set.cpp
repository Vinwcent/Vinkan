#include "descriptor_set.hpp"

#include <algorithm>
#include <cassert>

namespace vinkan {

DescriptorSet::Builder &DescriptorSet::Builder::setResource(
    BufferDescriptorInfo descriptorInfo) {
  assert(!descriptorInfo.vkResourceInfo.empty());
  assert(std::find(bindingIndices_.begin(), bindingIndices_.end(),
                   descriptorInfo.bindingIndex) == bindingIndices_.end() &&
         "There's already something on this binding");

  auto layoutBinding = setLayout_.getLayoutBinding(descriptorInfo.bindingIndex);
  assert(layoutBinding.descriptorCount == descriptorInfo.vkResourceInfo.size());

  bufferInfos_.push_back(descriptorInfo.vkResourceInfo);
  auto &ownedBufferInfo = bufferInfos_[bufferInfos_.size() - 1];

  VkWriteDescriptorSet setWrite{};
  setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  setWrite.descriptorType = layoutBinding.descriptorType;
  setWrite.dstBinding = descriptorInfo.bindingIndex;
  setWrite.pBufferInfo = ownedBufferInfo.data();
  setWrite.descriptorCount = ownedBufferInfo.size();

  bindingIndices_.push_back(descriptorInfo.bindingIndex);
  setWrites_.push_back(setWrite);
  return *this;
}

DescriptorSet::Builder &DescriptorSet::Builder::setResource(
    ImageDescriptorInfo descriptorInfo) {
  assert(!descriptorInfo.vkResourceInfo.empty());
  assert(std::find(bindingIndices_.begin(), bindingIndices_.end(),
                   descriptorInfo.bindingIndex) == bindingIndices_.end() &&
         "There's already something on this binding");

  auto layoutBinding = setLayout_.getLayoutBinding(descriptorInfo.bindingIndex);
  assert(layoutBinding.descriptorCount == descriptorInfo.vkResourceInfo.size());

  imageInfos_.push_back(descriptorInfo.vkResourceInfo);
  auto &ownedImageInfo = imageInfos_[imageInfos_.size() - 1];

  VkWriteDescriptorSet setWrite{};
  setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  setWrite.descriptorType = layoutBinding.descriptorType;
  setWrite.dstBinding = descriptorInfo.bindingIndex;
  setWrite.pImageInfo = ownedImageInfo.data();
  setWrite.descriptorCount = ownedImageInfo.size();

  bindingIndices_.push_back(descriptorInfo.bindingIndex);
  setWrites_.push_back(setWrite);
  return *this;
}

std::unique_ptr<DescriptorSet> DescriptorSet::Builder::build() {
  VkDescriptorSet descriptorSet{};
  bool success = pool_.allocateDescriptorSet(
      setLayout_.getDescriptorSetLayout(), descriptorSet);
  if (!success) {
    throw std::runtime_error("Could not allocate the descriptor set");
  }
  for (auto &setWrite : setWrites_) {
    setWrite.dstSet = descriptorSet;
  }

  vkUpdateDescriptorSets(device_, setWrites_.size(), setWrites_.data(), 0,
                         nullptr);
  return std::unique_ptr<DescriptorSet>(new DescriptorSet(descriptorSet));
}

void DescriptorSet::Builder::build(DescriptorSet &descriptorSet) {
  for (auto &setWrite : setWrites_) {
    setWrite.dstSet = descriptorSet.getHandle();
  }
  vkUpdateDescriptorSets(device_, setWrites_.size(), setWrites_.data(), 0,
                         nullptr);
}

DescriptorSet::DescriptorSet(VkDescriptorSet handle) { handle_ = handle; }

}  // namespace vinkan
