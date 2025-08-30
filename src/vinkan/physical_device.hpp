#ifndef VINKAN_PHYSICAL_DEVICE_HPP
#define VINKAN_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan.h>

#include <cassert>
#include <optional>
#include <set>
#include <vector>

#include "vinkan/generics/ptr_handle_wrapper.hpp"
#include "vinkan/structs/queue_family_info.hpp"

namespace vvw_vk {

struct PhysicalDeviceInfo {
  std::vector<VkQueueFlagBits> requestedQueueFlags;
  std::optional<VkSurfaceKHR> surfaceSupportRequested;
  std::set<const char *> extensions;
};

struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevice : public PtrHandleWrapper<VkPhysicalDevice> {
 public:
  PhysicalDevice(PhysicalDeviceInfo, VkInstance);

  // This function will return the surface support details of the surface used
  // to select this physical device.
  SurfaceSupportDetails getSurfaceSupportDetails();
  std::vector<QueueFamilyInfo> getQueues();

 private:
  bool withSurfaceSupport = false;
  VkSurfaceKHR surface_ = nullptr;

  std::vector<VkPhysicalDevice> getVkCompatibleDevice_(VkInstance);

  bool isSuitable_(VkPhysicalDevice, PhysicalDeviceInfo) const;
  bool supportExtensions_(VkPhysicalDevice physicalDevice,
                          const std::set<const char *> &extensions) const;
  SurfaceSupportDetails getSurfaceSupportDetails_(
      VkPhysicalDevice physicalDevice) const;

  std::vector<QueueFamilyInfo> getQueuesInfo_(
      VkPhysicalDevice physicalDevice) const;
};
}  // namespace vvw_vk

#endif /* PHYSICAL_DEVICE_H */

