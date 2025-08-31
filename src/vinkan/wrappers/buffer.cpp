#include "buffer.hpp"

// std
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace vinkan {

uint32_t getMemoryTypeIndex(
    uint32_t typeFilter,
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
    VkMemoryPropertyFlags bufferMemoryPropFlags) {
  for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (deviceMemoryProperties.memoryTypes[i].propertyFlags &
         bufferMemoryPropFlags) == bufferMemoryPropFlags) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize,
                                  VkDeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}  // namespace

Buffer::Buffer(VkDevice device,
               VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
               BufferInfo bufferInfo)
    : device_(device),
      instanceSize{bufferInfo.instanceSize},
      instanceCount{bufferInfo.instanceCount},
      usageFlags{bufferInfo.usageFlags},
      memoryPropertyFlags{bufferInfo.memoryPropertyFlags} {
  alignmentSize = getAlignment(instanceSize, bufferInfo.minOffsetAlignment);
  bufferSize = alignmentSize * instanceCount;

  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = bufferSize;
  bufferCreateInfo.usage = bufferInfo.usageFlags;
  bufferCreateInfo.sharingMode = bufferInfo.sharingMode.value;
  if (bufferInfo.sharingMode.value != VK_SHARING_MODE_EXCLUSIVE) {
    assert(bufferInfo.sharingMode.concurrentQueueFamilies.has_value());
    bufferCreateInfo.queueFamilyIndexCount =
        bufferInfo.sharingMode.concurrentQueueFamilies->size();
    bufferCreateInfo.pQueueFamilyIndices =
        bufferInfo.sharingMode.concurrentQueueFamilies->data();
  }

  if (vkCreateBuffer(device_, &bufferCreateInfo, nullptr, &handle_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer !");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device_, handle_, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      getMemoryTypeIndex(memRequirements.memoryTypeBits, deviceMemoryProperties,
                         bufferInfo.memoryPropertyFlags);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(device_, handle_, memory_, 0);
}

Buffer::~Buffer() {
  if (isHandleValid()) {
    unmap();
    vkDestroyBuffer(device_, handle_, nullptr);
    vkFreeMemory(device_, memory_, nullptr);
  }
}

VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
  assert(handle_ && memory_ && "Called map on buffer before create");
  return vkMapMemory(device_, memory_, offset, size, 0, &mapped);
}

void Buffer::unmap() {
  if (mapped) {
    vkUnmapMemory(device_, memory_);
    mapped = nullptr;
  }
}

void Buffer::readBuffer(void *data) {
  assert(mapped && "Cannot copy to unmapped buffer");
  memcpy(data, mapped, bufferSize);
}

void Buffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
  assert(mapped && "Cannot copy to unmapped buffer");

  if (size == VK_WHOLE_SIZE) {
    memcpy(mapped, data, bufferSize);
  } else {
    char *memOffset = (char *)mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = memory_;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkFlushMappedMemoryRanges(device_, 1, &mappedRange);
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = memory_;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkInvalidateMappedMemoryRanges(device_, 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size,
                                              VkDeviceSize offset) {
  return VkDescriptorBufferInfo{
      handle_,
      offset,
      size,
  };
}

void Buffer::writeToIndex(void *data, int index) {
  writeToBuffer(data, instanceSize, index * alignmentSize);
}

VkResult Buffer::flushIndex(int index) {
  return flush(alignmentSize, index * alignmentSize);
}

VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
  return descriptorInfo(alignmentSize, index * alignmentSize);
}

VkResult Buffer::invalidateIndex(int index) {
  return invalidate(alignmentSize, index * alignmentSize);
}

}  // namespace vinkan
