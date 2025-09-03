#include "swapchain.hpp"

#include <cassert>
#include <stdexcept>

#include "vinkan/logging/logger.hpp"

namespace vinkan {
Swapchain::Swapchain(SwapchainInfo swapchainInfo) {
  assert(swapchainInfo.surfaceSupportDetails
             .findBestFormat({swapchainInfo.surfaceFormat})
             .has_value() &&
         "Surface doesn't support the selected format");
  assert(swapchainInfo.surfaceSupportDetails
             .findBestPresentMode({swapchainInfo.presentMode})
             .has_value() &&
         "Surface doesn't support the selected present mode");
  assert(swapchainInfo.imageCount <=
             swapchainInfo.surfaceSupportDetails.capabilities.maxImageCount &&
         "Surface can't support the selected image count");

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = swapchainInfo.surface;

  createInfo.minImageCount = swapchainInfo.imageCount;
  createInfo.imageFormat = swapchainInfo.surfaceFormat.format;
  createInfo.imageColorSpace = swapchainInfo.surfaceFormat.colorSpace;
  createInfo.imageExtent = swapchainInfo.imageExtent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  auto sharingMode = swapchainInfo.sharingMode.value;
  createInfo.imageSharingMode = swapchainInfo.sharingMode.value;
  if (sharingMode == VK_SHARING_MODE_CONCURRENT) {
    assert(swapchainInfo.sharingMode.concurrentQueueFamilies.has_value() &&
           swapchainInfo.sharingMode.concurrentQueueFamilies->size() > 1);
    createInfo.queueFamilyIndexCount =
        swapchainInfo.sharingMode.concurrentQueueFamilies->size();
    createInfo.pQueueFamilyIndices =
        swapchainInfo.sharingMode.concurrentQueueFamilies->data();
  }

  createInfo.preTransform =
      swapchainInfo.surfaceSupportDetails.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = swapchainInfo.presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(swapchainInfo.device, &createInfo, nullptr,
                           &handle_) != VK_SUCCESS) {
    throw std::runtime_error("Could not create the swapchain");
  }
  device_ = swapchainInfo.device;
  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Swapchain created");
}

Swapchain::~Swapchain() {
  if (isHandleValid()) {
    vkDestroySwapchainKHR(device_, handle_, nullptr);
  }
}

std::optional<uint32_t> Swapchain::acquireNextImageIndex(
    VkSemaphore semaphoreToSignal, VkFence fenceToSignal) {
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      device_, handle_, std::numeric_limits<uint64_t>::max(), semaphoreToSignal,
      fenceToSignal, &imageIndex);
  if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
    return imageIndex;
  }
  return std::nullopt;
}

}  // namespace vinkan
