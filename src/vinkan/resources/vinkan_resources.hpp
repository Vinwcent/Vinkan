#ifndef VINKAN_VINKAN_RESOURCES_HPP
#define VINKAN_VINKAN_RESOURCES_HPP
#include <vulkan/vulkan.h>

#include <map>
#include <memory>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/resources/resources_binder.hpp"
#include "vinkan/wrappers/buffer.hpp"

namespace vinkan {

template <EnumType BufferT>
struct VinkanBufferBinding {
  uint32_t bindingIndex;
  BufferT buffer;
};

template <EnumType BufferT, EnumType SetT, EnumType SetLayoutT, EnumType PoolT>
class VinkanResources {
 public:
  VinkanResources(VkDevice device,
                  VkPhysicalDeviceMemoryProperties deviceMemoryProperties)
      : device_(device),
        deviceMemoryProperties_(deviceMemoryProperties),
        resourcesBinder_(device) {}

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

  void createPool(PoolT pool,
                  const std::vector<SetLayoutT> &setLayoutIdentifiers) {
    resourcesBinder_.createPool(pool, setLayoutIdentifiers);
  }
  void createSetLayout(SetLayoutT setLayout, SetLayoutInfo layoutInfo) {
    resourcesBinder_.createSetLayout(setLayout, layoutInfo);
  }

  void createSet(SetT setIdentifier, SetLayoutT setLayoutIdentifier,
                 std::vector<VinkanBufferBinding<BufferT>> bufferBindings) {
    std::vector<ResourceDescriptorInfo> resourceDescriptorInfos{};
    for (auto &bufferBinding : bufferBindings) {
      assert(buffers_.contains(bufferBinding.buffer));
      auto &buffer = buffers_.at(bufferBinding.buffer);
      ResourceDescriptorInfo resourceDescriptorInfo{
          .bindingIndex = bufferBinding.bindingIndex,
          .vkBufferInfo = {buffer->descriptorInfo()}};
      resourceDescriptorInfos.push_back(resourceDescriptorInfo);
    }
    resourcesBinder_.createSet(setIdentifier, setLayoutIdentifier,
                               resourceDescriptorInfos);
  }

 private:
  std::map<BufferT, std::unique_ptr<Buffer>> buffers_;

  VkDevice device_;
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties_;
  ResourcesBinder<SetT, SetLayoutT, PoolT> resourcesBinder_;
};
}  // namespace vinkan
#endif
