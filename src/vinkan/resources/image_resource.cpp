#include "image_resource.hpp"

#include "vinkan/logging/logger.hpp"

namespace vinkan {

ImageResource::ImageResource(ImageView& imageView, Sampler& sampler)
    : imageView_(imageView), sampler_(&sampler) {
  SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                     "ImageResource created (with sampler)");
}

ImageResource::ImageResource(ImageView& imageView)
    : imageView_(imageView), sampler_(nullptr) {
  SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                     "ImageResource created (storage image)");
}

VkDescriptorImageInfo ImageResource::descriptorInfo(VkImageLayout layout) {
  return VkDescriptorImageInfo{
      sampler_ ? sampler_->getHandle() : VK_NULL_HANDLE,
      imageView_.getHandle(),
      layout,
  };
}

}  // namespace vinkan
