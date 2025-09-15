#ifndef VINKAN_IMAGE_VIEW_HPP
#define VINKAN_IMAGE_VIEW_HPP

#include <vulkan/vulkan.h>

#include "vinkan/generics/ptr_handle_wrapper.hpp"

namespace vinkan {

struct ImageViewInfo {
  VkImage image;
  VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
  VkFormat format;
  VkComponentMapping components = {
      VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
      VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
  VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
                                              VK_REMAINING_MIP_LEVELS, 0,
                                              VK_REMAINING_ARRAY_LAYERS};
};

class ImageView : public PtrHandleWrapper<VkImageView> {
 public:
  ImageView(VkDevice device, ImageViewInfo imageViewInfo);
  ~ImageView();

  ImageView(const ImageView&) = delete;
  ImageView& operator=(const ImageView&) = delete;

  VkImage getImage() const { return image_; }
  VkImageViewType getViewType() const { return viewType; }
  VkFormat getFormat() const { return format; }
  VkComponentMapping getComponents() const { return components; }
  VkImageSubresourceRange getSubresourceRange() const {
    return subresourceRange;
  }

 private:
  VkDevice device_;
  VkImage image_;
  VkImageViewType viewType;
  VkFormat format;
  VkComponentMapping components;
  VkImageSubresourceRange subresourceRange;
};

}  // namespace vinkan

#endif
