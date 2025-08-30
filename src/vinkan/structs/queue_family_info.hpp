#ifndef VINKAN_QUEUE_FAMILY_INFO_HPP
#define VINKAN_QUEUE_FAMILY_INFO_HPP

#include <vulkan/vulkan_core.h>

#include <cassert>

namespace vinkan {

struct QueueFamilyInfo {
 public:
  uint32_t queueIndex;
  uint32_t queueCount;
  uint32_t queueFlags;

  QueueFamilyInfo(uint32_t queueIndex, uint32_t queueCount, uint32_t queueFlags,
                  VkPhysicalDevice physicalDevice)
      : queueIndex(queueIndex),
        queueCount(queueCount),
        queueFlags(queueFlags),
        physicalDevice_(physicalDevice)

  {}

  bool supportQueueFlags(uint32_t flags) { return flags & queueFlags; }
  bool supportCompute() { return supportQueueFlags(VK_QUEUE_COMPUTE_BIT); }
  bool supportGfx() { return supportQueueFlags(VK_QUEUE_GRAPHICS_BIT); }
  bool supportPresentation(VkSurfaceKHR surface) const {
    VkBool32 presentSupport;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, queueIndex, surface,
                                         &presentSupport);
    return presentSupport;
  };

 private:
  VkPhysicalDevice physicalDevice_;
};

}  // namespace vinkan

#endif /* QUEUE_FAMILY_INDICES_H */

