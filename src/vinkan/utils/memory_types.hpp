#ifndef VINKAN_MEMORY_TYPES_HPP
#define VINKAN_MEMORY_TYPES_HPP

#include <stdexcept>
#include <vulkan/vulkan_core.h>

inline uint32_t
getMemoryTypeIndex(uint32_t typeFilter,
                   VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
                   VkMemoryPropertyFlags bufferMemoryPropFlags) {
  for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (deviceMemoryProperties.memoryTypes[i].propertyFlags &
         bufferMemoryPropFlags) == bufferMemoryPropFlags) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

#endif
