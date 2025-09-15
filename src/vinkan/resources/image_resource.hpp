#ifndef VINKAN_IMAGE_RESOURCE_HPP
#define VINKAN_IMAGE_RESOURCE_HPP

#include <vulkan/vulkan.h>

#include "vinkan/wrappers/image_view.hpp"
#include "vinkan/wrappers/sampler.hpp"

namespace vinkan {

class ImageResource {
 public:
  ImageResource(ImageView& imageView, Sampler& sampler);
  ImageResource(ImageView& imageView);

  ~ImageResource() = default;

  ImageResource(const ImageResource&) = delete;
  ImageResource& operator=(const ImageResource&) = delete;

  VkDescriptorImageInfo descriptorInfo(
      VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

 private:
  ImageView& imageView_;
  Sampler* sampler_;
};

}  // namespace vinkan

#endif
