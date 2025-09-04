#include "swapchain.hpp"

#include <cassert>
#include <stdexcept>

#include "vinkan/logging/logger.hpp"

namespace vinkan {
Swapchain::Swapchain(SwapchainInfo swapchainInfo)
    : swapchainInfo_(swapchainInfo) {
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
  for (auto imageView : imageViews_) {
    vkDestroyImageView(device_, imageView, nullptr);
  }

  if (isHandleValid()) {
    vkDestroySwapchainKHR(device_, handle_, nullptr);
  }
}
SwapchainInfo Swapchain::getSwapchainInfo() { return swapchainInfo_; }

std::vector<VkImageView> Swapchain::getImageViews() {
  if (imageViews_.size() > 0) {
    return imageViews_;
  }
  std::vector<VkImage> images = {};
  uint32_t realImageCount;
  vkGetSwapchainImagesKHR(device_, handle_, &realImageCount, nullptr);
  images.resize(realImageCount);
  vkGetSwapchainImagesKHR(device_, handle_, &realImageCount, images.data());

  imageViews_.resize(images.size());
  for (size_t i = 0; i < imageViews_.size(); i++) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = images[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapchainInfo_.surfaceFormat.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device_, &viewInfo, nullptr, &imageViews_[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
  return imageViews_;
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
