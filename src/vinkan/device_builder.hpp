#ifndef VINKAN_DEVICE_BUILDER_HPP
#define VINKAN_DEVICE_BUILDER_HPP

#include <vulkan/vulkan.h>

#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "vinkan/device.hpp"
#include "vinkan/logging/logger.hpp"
#include "vinkan/structs/queue_family_info.hpp"

namespace vinkan {

template <EnumType T>
struct QueueFamilyRequest {
  T queueFamilyIdentifier;

  uint32_t flagsRequested;
  std::optional<VkSurfaceKHR> surfacePresentationSupport;
  uint32_t nQueues;
  std::vector<float> queuePriorities;
};

template <EnumType T>
class DeviceBuilder {
 public:
  DeviceBuilder(VkPhysicalDevice physicalDevice,
                std::vector<QueueFamilyInfo> queuesInfo)
      : physicalDevice_(physicalDevice), queuesInfo_(queuesInfo) {}

  void addExtensions(const std::set<const char *> deviceExtensions) {
    deviceExtensions_.insert(deviceExtensions.begin(), deviceExtensions.end());
  }
  void addQueue(QueueFamilyRequest<T> &queueRequest, bool differentFromPrevious,
                bool &success) {
    assert(queueRequest.queuePriorities.size() == queueRequest.nQueues);
    std::optional<QueueFamilyInfo> selectedQueueOpt = std::nullopt;
    for (auto &queueInfo : queuesInfo_) {
      // If we asked for a different and we have it, we continue
      if (differentFromPrevious && isPreviousQueue_(queueInfo)) {
        continue;
      }
      // If it fits the request, we're done
      if (queueFitRequest_(queueInfo, queueRequest)) {
        selectedQueueOpt = queueInfo;
        break;
      }
    }
    // If we didn't find any, we go out
    if (!selectedQueueOpt.has_value()) {
      success = false;
      return;
    }
    auto selectedQueue = selectedQueueOpt.value();
    familyIdentifierToAllocInfo_[queueRequest.queueFamilyIdentifier] = {
        selectedQueue.queueIndex, queueRequest.nQueues};
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = selectedQueue.queueIndex;
    queueCreateInfo.queueCount = queueRequest.nQueues;
    queueCreateInfo.pQueuePriorities = queueRequest.queuePriorities.data();
    queueCreateInfo_.push_back(queueCreateInfo);
    success = true;
  }
  Device<T> build() {
    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.pNext = nullptr;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfo_.size());
    createInfo.pQueueCreateInfos = queueCreateInfo_.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.pNext = &features12;
    std::vector<const char *> deviceExtensionsVector(deviceExtensions_.begin(),
                                                     deviceExtensions_.end());
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensionsVector.size());
    createInfo.ppEnabledExtensionNames = deviceExtensionsVector.data();

    createInfo.enabledLayerCount = 0;

    VkDevice deviceHandle;
    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &deviceHandle) !=
        VK_SUCCESS) {
      throw std::runtime_error("Could not create the vulkan device");
    }
    auto device = Device<T>(deviceHandle, familyIdentifierToAllocInfo_);
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Device created !");
    return device;
  }

 private:
  std::vector<QueueFamilyInfo> queuesInfo_;

  std::map<T, AllocatedQueueFamilyInfo> familyIdentifierToAllocInfo_{};

  // Build info
  VkPhysicalDevice physicalDevice_;
  std::set<const char *> deviceExtensions_{};
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfo_{};

  bool isPreviousQueue_(QueueFamilyInfo queueInfo) const {
    for (auto previousQueueCreate : queueCreateInfo_) {
      if (queueInfo.queueIndex == previousQueueCreate.queueFamilyIndex) {
        return true;
      }
    }
    return false;
  }

  bool queueFitRequest_(QueueFamilyInfo queueInfo,
                        QueueFamilyRequest<T> queueRequest) const {
    bool validFlags = false;
    bool validSurface = false;
    bool validCount = false;
    if (queueInfo.supportQueueFlags(queueRequest.flagsRequested)) {
      validFlags = true;
    }
    if (queueInfo.queueCount >= queueRequest.nQueues) {
      validCount = true;
    }
    if (queueRequest.surfacePresentationSupport.has_value()) {
      validSurface = queueInfo.supportPresentation(
          queueRequest.surfacePresentationSupport.value());
    } else {
      // We valid because there's no surface to support
      validSurface = true;
    }
    return validSurface && validCount && validFlags;
  }
};

}  // namespace vinkan

#endif

