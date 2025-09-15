#include "image_view.hpp"

#include "vinkan/logging/logger.hpp"

// std
#include <stdexcept>

namespace vinkan {

ImageView::ImageView(VkDevice device, ImageViewInfo imageViewInfo)
    : device_(device),
      image_{imageViewInfo.image},
      viewType{imageViewInfo.viewType},
      format{imageViewInfo.format},
      components{imageViewInfo.components},
      subresourceRange{imageViewInfo.subresourceRange} {
  VkImageViewCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = imageViewInfo.image;
  createInfo.viewType = imageViewInfo.viewType;
  createInfo.format = imageViewInfo.format;
  createInfo.components = imageViewInfo.components;
  createInfo.subresourceRange = imageViewInfo.subresourceRange;

  if (vkCreateImageView(device_, &createInfo, nullptr, &handle_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view!");
  }

  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "ImageView created");
}

ImageView::~ImageView() {
  if (isHandleValid()) {
    vkDestroyImageView(device_, handle_, nullptr);
  }
}

}  // namespace vinkan
