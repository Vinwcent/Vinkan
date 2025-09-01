#ifndef VINKAN_DESCRIPTOR_SET_HPP
#define VINKAN_DESCRIPTOR_SET_HPP

#include "descriptor_pool.hpp"
#include "descriptor_set_layout.hpp"
#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

struct ResourceDescriptorInfo {
  uint32_t bindingIndex;
  std::vector<VkDescriptorBufferInfo> vkBufferInfo;
};

// This class is just a wrapper around the handle, only the builder is really
// useful. I keep it for API consistency.
class DescriptorSet : public PtrHandleWrapper<VkDescriptorSet> {
 public:
  class Builder {
   public:
    Builder(VkDevice device, DescriptorSetLayout &setLayout,
            DescriptorPool &pool)
        : device_(device), setLayout_(setLayout), pool_(pool) {}
    Builder &setBuffer(ResourceDescriptorInfo descriptorInfo);
    std::unique_ptr<DescriptorSet> build();
    void build(DescriptorSet &descriptorSet);

   private:
    std::vector<std::vector<VkDescriptorBufferInfo>>
        bufferInfos_{};  // For memory
                         // purpose
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
