#ifndef VINKAN_DEVICE_HPP
#define VINKAN_DEVICE_HPP

#include <vulkan/vulkan.h>

#include <map>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

template <EnumType T>
class DeviceBuilder;

struct AllocatedQueueFamilyInfo {
  uint32_t queueFamilyIndex;  // Here it's the vulkan identifier
  uint32_t queueCount;        // Here it's the number of queues
};

template <typename T>
class Device : public PtrHandleWrapper<VkDevice> {
 public:
  VkQueue getQueue(T queueIdentifier, uint32_t queueNumber) {
    auto& allocInfo = familyIdentifierToAllocInfo_[queueIdentifier];
    assert(queueNumber < allocInfo.queueCount);
    VkQueue queue;
    vkGetDeviceQueue(handle_, allocInfo.queueFamilyIndex, queueNumber, &queue);
    return queue;
  }

  uint32_t getQueueFamilyIndex(T queueIdentifier) {
    auto& allocInfo = familyIdentifierToAllocInfo_[queueIdentifier];
    return allocInfo.queueFamilyIndex;
  }

  ~Device() {
    if (isHandleValid()) {
      vkDestroyDevice(handle_, nullptr);
    }
  }

  Device(Device&& other) noexcept
      : familyIdentifierToAllocInfo_(
            std::move(other.familyIdentifierToAllocInfo_)) {
    handle_ = other.handle_;
    other.handle_ = VK_NULL_HANDLE;
  }

  // Move assignment
  Device& operator=(Device&& other) noexcept {
    if (this != &other) {
      if (handle_ != VK_NULL_HANDLE) {
        vkDestroyDevice(handle_, nullptr);
      }
      handle_ = other.handle_;
      familyIdentifierToAllocInfo_ =
          std::move(other.familyIdentifierToAllocInfo_);
      other.handle_ = VK_NULL_HANDLE;
    }
    return *this;
  }

 private:
  std::map<T, AllocatedQueueFamilyInfo> familyIdentifierToAllocInfo_{};

  Device(VkDevice device,
         std::map<T, AllocatedQueueFamilyInfo> familyIdentifierToAllocInfo)
      : familyIdentifierToAllocInfo_(familyIdentifierToAllocInfo) {
    handle_ = device;
  }

  friend class DeviceBuilder<T>;
};

}  // namespace vinkan

#endif

