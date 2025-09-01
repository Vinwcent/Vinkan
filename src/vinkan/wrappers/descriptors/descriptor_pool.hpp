#ifndef VINKAN_DESCRIPTOR_POOL_HPP
#define VINKAN_DESCRIPTOR_POOL_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace vinkan {

class DescriptorPool {
 public:
  class Builder {
   public:
    Builder(VkDevice device) : device_{device} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPool> build() const;

   private:
    VkDevice device_;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  ~DescriptorPool();
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

  bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout,
                             VkDescriptorSet &descriptor) const;
  void freeDescriptors(const std::vector<VkDescriptorSet> &descriptors) const;
  void resetPool();

 private:
  DescriptorPool(VkDevice device, uint32_t maxSets,
                 VkDescriptorPoolCreateFlags poolFlags,
                 const std::vector<VkDescriptorPoolSize> &poolSizes);
  VkDevice device_;
  VkDescriptorPool descriptorPool;

  friend class Builder;
};

}  // namespace vinkan

#endif
