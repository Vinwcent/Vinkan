#ifndef VINKAN_VINKAN_RESOURCES_HPP
#define VINKAN_VINKAN_RESOURCES_HPP
#include <vulkan/vulkan.h>

#include <map>
#include <memory>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/wrappers/buffer.hpp"

namespace vinkan {

template <EnumType BufferT>
class VinkanResources {
 public:
  VinkanResources(VkDevice device,
                  VkPhysicalDeviceMemoryProperties deviceMemoryProperties)
      : device_(device), deviceMemoryProperties_(deviceMemoryProperties) {}

  void create(BufferT bufferIdentifier, BufferInfo bufferInfo) {
    assert(!buffers_.contains(bufferIdentifier));
    buffers_.emplace(
        bufferIdentifier,
        std::make_unique<Buffer>(device_, deviceMemoryProperties_, bufferInfo));
  }

  Buffer &get(BufferT bufferIdentifier) {
    assert(buffers_.contains(bufferIdentifier));
    return *buffers_[bufferIdentifier];
  }

 private:
  std::map<BufferT, std::unique_ptr<Buffer>> buffers_;

  VkDevice device_;
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties_;
};
}  // namespace vinkan
#endif
