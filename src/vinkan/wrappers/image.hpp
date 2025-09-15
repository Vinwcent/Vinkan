#ifndef VINKAN_IMAGE_HPP
#define VINKAN_IMAGE_HPP

#include <vulkan/vulkan.h>

#include "vinkan/generics/ptr_handle_wrapper.hpp"
#include "vinkan/structs/sharing_mode.hpp"

namespace vinkan {

struct ImageInfo {
  VkImageType imageType = VK_IMAGE_TYPE_2D;
  VkFormat format;
  VkExtent3D extent;
  uint32_t mipLevels = 1;
  uint32_t arrayLayers = 1;
  VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags usageFlags;
  SharingMode sharingMode;
  VkMemoryPropertyFlags memoryPropertyFlags;
  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class Image : public PtrHandleWrapper<VkImage> {
 public:
  Image(VkDevice device,
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
        ImageInfo imageInfo);
  ~Image();

  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  VkFormat getFormat() const { return format; }
  VkExtent3D getExtent() const { return extent; }
  uint32_t getMipLevels() const { return mipLevels; }
  uint32_t getArrayLayers() const { return arrayLayers; }
  VkSampleCountFlagBits getSamples() const { return samples; }
  VkImageUsageFlags getUsageFlags() const { return usageFlags; }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const {
    return memoryPropertyFlags;
  }
  VkImageType getImageType() const { return imageType; }
  VkImageTiling getTiling() const { return tiling; }
  VkDeviceSize getImageSize() const { return imageSize; }
  VkDeviceMemory getMemory() const { return memory_; }

 private:
  VkDevice device_;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;

  VkFormat format;
  VkExtent3D extent;
  uint32_t mipLevels;
  uint32_t arrayLayers;
  VkSampleCountFlagBits samples;
  VkImageUsageFlags usageFlags;
  VkMemoryPropertyFlags memoryPropertyFlags;
  VkImageType imageType;
  VkImageTiling tiling;
  VkDeviceSize imageSize;
};

}  // namespace vinkan

#endif
