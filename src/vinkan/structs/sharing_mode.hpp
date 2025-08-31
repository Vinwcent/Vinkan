#ifndef VINKAN_SHARING_MODE_HPP
#define VINKAN_SHARING_MODE_HPP

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace vinkan {
struct SharingMode {
  VkSharingMode value;
  std::optional<std::vector<uint32_t>> concurrentQueueFamilies;
};

}  // namespace vinkan

#endif
