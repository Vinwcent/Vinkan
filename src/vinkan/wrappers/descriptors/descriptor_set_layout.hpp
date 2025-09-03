#ifndef VINKAN_DESCRIPTOR_SET_LAYOUT_HPP
#define VINKAN_DESCRIPTOR_SET_LAYOUT_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <unordered_map>

#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

struct DescriptorSetLayoutBinding {
  uint32_t bindingIndex;
  VkDescriptorType descriptorType;
  VkShaderStageFlags shaderStageFlags;
  uint32_t count = 1;
};

class DescriptorSetLayout : public PtrHandleWrapper<VkDescriptorSetLayout> {
 public:
  class Builder {
   public:
    Builder(VkDevice device) : device_(device) {}

    Builder &addBinding(DescriptorSetLayoutBinding descriptorSetLayoutBinding);
    std::unique_ptr<DescriptorSetLayout> build() const;

   private:
    VkDevice device_;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> vkBindings_{};
  };

  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return handle_; }

  VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t bindingIndex) const;

 private:
  DescriptorSetLayout(
      VkDevice device,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  VkDevice device_;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

  friend class Builder;
};

}  // namespace vinkan

#endif
