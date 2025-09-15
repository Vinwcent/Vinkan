#ifndef VINKAN_DESCRIPTOR_SET_HPP
#define VINKAN_DESCRIPTOR_SET_HPP

#include <type_traits>

#include "descriptor_pool.hpp"
#include "descriptor_set_layout.hpp"
#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

template <typename T>
concept ValidDescriptorInfo = std::is_same_v<T, VkDescriptorBufferInfo> ||
                              std::is_same_v<T, VkDescriptorImageInfo>;

template <ValidDescriptorInfo T>
struct ResourceDescriptorInfo {
  uint32_t bindingIndex;
  std::vector<T> vkResourceInfo;
};

using BufferDescriptorInfo = ResourceDescriptorInfo<VkDescriptorBufferInfo>;
using ImageDescriptorInfo = ResourceDescriptorInfo<VkDescriptorImageInfo>;

class DescriptorSet : public PtrHandleWrapper<VkDescriptorSet> {
 public:
  class Builder {
   public:
    Builder(VkDevice device, DescriptorSetLayout &setLayout,
            DescriptorPool &pool)
        : device_(device), setLayout_(setLayout), pool_(pool) {}

    Builder &setResource(BufferDescriptorInfo descriptorInfo);
    Builder &setResource(ImageDescriptorInfo descriptorInfo);

    std::unique_ptr<DescriptorSet> build();
    void build(DescriptorSet &descriptorSet);

   private:
    std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos_{};
    std::vector<std::vector<VkDescriptorImageInfo>> imageInfos_{};
    std::vector<uint32_t> bindingIndices_{};
    std::vector<VkWriteDescriptorSet> setWrites_;
    DescriptorSetLayout &setLayout_;
    DescriptorPool &pool_;
    VkDevice device_;
  };

 private:
  DescriptorSet(VkDescriptorSet handle);
  friend class Builder;
};

}  // namespace vinkan

#endif
