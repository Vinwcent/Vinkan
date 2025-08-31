#ifndef VINKAN_SURFACE_SUPPORT_DETAILS_HPP
#define VINKAN_SURFACE_SUPPORT_DETAILS_HPP

#include <vulkan/vulkan.h>

#include <algorithm>
#include <limits>
#include <optional>
#include <vector>

template <typename T, typename Predicate>
std::optional<T> findCompatiblePreferredMatch(const std::vector<T>& preferred,
                                              const std::vector<T>& available,
                                              Predicate pred) {
  for (const auto& pref : preferred) {
    for (const auto& avail : available) {
      if (pred(pref, avail)) {
        return avail;
      }
    }
  }
  return std::nullopt;
};

struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;

  /**
   * @brief Find the most preferred format that is compatible with the surface
   * details. Returns null if there's no format compatible
   *
   */
  std::optional<VkSurfaceFormatKHR> findBestFormat(
      const std::vector<VkSurfaceFormatKHR>& preferredFormats) const {
    return findCompatiblePreferredMatch(
        preferredFormats, formats, [](const auto& a, const auto& b) {
          return a.format == b.format && a.colorSpace == b.colorSpace;
        });
  }

  /**
   * @brief Find the most preferred mode that is compatible with the surface
   * details. Returns null if there's no present mode compatible
   *
   */
  std::optional<VkPresentModeKHR> findBestPresentMode(
      const std::vector<VkPresentModeKHR>& preferredModes) const {
    return findCompatiblePreferredMatch(
        preferredModes, presentModes,
        [](const auto& a, const auto& b) { return a == b; });
  }

  /**
   * @brief Find a compatible extent with the window extent
   *
   */
  VkExtent2D selectExtent(VkExtent2D windowExtent) const {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    }

    return {.width = std::clamp(windowExtent.width,
                                capabilities.minImageExtent.width,
                                capabilities.maxImageExtent.width),
            .height = std::clamp(windowExtent.height,
                                 capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)};
  }
};

#endif
