#ifndef VINKAN_SWAPCHAIN_HPP
#define VINKAN_SWAPCHAIN_HPP

#include <vulkan/vulkan.h>

#include <optional>

#include "vinkan/generics/ptr_handle_wrapper.hpp"
#include "vinkan/structs/sharing_mode.hpp"
#include "vinkan/structs/surface_support_details.hpp"

namespace vinkan {

struct SwapchainInfo {
  VkDevice device;
  VkSurfaceKHR surface;
  VkExtent2D imageExtent;
  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR presentMode;
  uint32_t imageCount;
  SharingMode sharingMode;

  SurfaceSupportDetails surfaceSupportDetails;
};

class Swapchain : public PtrHandleWrapper<VkSwapchainKHR> {
 public:
  Swapchain(SwapchainInfo swapchainInfo);
  ~Swapchain();

  std::optional<uint32_t> acquireNextImageIndex(
      VkSemaphore semaphoreToSignal, VkFence fenceToSignal = VK_NULL_HANDLE);

 private:
  VkDevice device_ = nullptr;
};
}  // namespace vinkan

#endif
