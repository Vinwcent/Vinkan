#ifndef VINKAN_RENDER_PASS_HPP
#define VINKAN_RENDER_PASS_HPP

#include <vulkan/vulkan.h>

#include <map>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {
template <EnumType T>
class RenderPassBuilder;

template <typename T>
class RenderPass : public PtrHandleWrapper<VkRenderPass> {
 public:
  ~RenderPass() {
    if (isHandleValid()) {
      vkDestroyRenderPass(device_, handle_, nullptr);
    }
  }

  RenderPass(RenderPass&& other) noexcept
      : attachmentIndices_(std::move(other.attachmentIndices_)) {
    handle_ = other.handle_;
    other.handle_ = VK_NULL_HANDLE;
  }

  // Move assignment
  RenderPass& operator=(RenderPass&& other) noexcept {
    if (this != &other) {
      if (handle_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, handle_, nullptr);
      }
      handle_ = other.handle_;
      attachmentIndices_ = std::move(other.attachmentIndices_);
      other.handle_ = VK_NULL_HANDLE;
    }
    return *this;
  }

 private:
  RenderPass(VkDevice device, VkRenderPass renderPass,
             std::map<T, uint32_t> attachmentIndices)
      : attachmentIndices_(attachmentIndices), device_(device) {
    handle_ = renderPass;
  }
  std::map<T, uint32_t> attachmentIndices_;
  VkDevice device_;

  friend class RenderPassBuilder<T>;
};
}  // namespace vinkan
#endif
