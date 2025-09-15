#include "image.hpp"

#include "vinkan/logging/logger.hpp"
#include "vinkan/utils/memory_types.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace vinkan {

Image::Image(VkDevice device,
             VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
             ImageInfo imageInfo)
    : device_(device),
      format{imageInfo.format},
      extent{imageInfo.extent},
      mipLevels{imageInfo.mipLevels},
      arrayLayers{imageInfo.arrayLayers},
      samples{imageInfo.samples},
      usageFlags{imageInfo.usageFlags},
      memoryPropertyFlags{imageInfo.memoryPropertyFlags},
      imageType{imageInfo.imageType},
      tiling{imageInfo.tiling} {
  VkImageCreateInfo imageCreateInfo{};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = imageInfo.imageType;
  imageCreateInfo.extent = imageInfo.extent;
  imageCreateInfo.mipLevels = imageInfo.mipLevels;
  imageCreateInfo.arrayLayers = imageInfo.arrayLayers;
  imageCreateInfo.format = imageInfo.format;
  imageCreateInfo.tiling = imageInfo.tiling;
  imageCreateInfo.initialLayout = imageInfo.initialLayout;
  imageCreateInfo.usage = imageInfo.usageFlags;
  imageCreateInfo.samples = imageInfo.samples;
  imageCreateInfo.sharingMode = imageInfo.sharingMode.value;

  if (imageInfo.sharingMode.value != VK_SHARING_MODE_EXCLUSIVE) {
    assert(imageInfo.sharingMode.concurrentQueueFamilies.has_value());
    imageCreateInfo.queueFamilyIndexCount =
        imageInfo.sharingMode.concurrentQueueFamilies->size();
    imageCreateInfo.pQueueFamilyIndices =
        imageInfo.sharingMode.concurrentQueueFamilies->data();
  }

  if (vkCreateImage(device_, &imageCreateInfo, nullptr, &handle_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_, handle_, &memRequirements);

  imageSize = memRequirements.size;

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      getMemoryTypeIndex(memRequirements.memoryTypeBits, deviceMemoryProperties,
                         imageInfo.memoryPropertyFlags);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(device_, handle_, memory_, 0);
  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Image created");
}

Image::~Image() {
  if (isHandleValid()) {
    vkDestroyImage(device_, handle_, nullptr);
    vkFreeMemory(device_, memory_, nullptr);
  }
}

}  // namespace vinkan
