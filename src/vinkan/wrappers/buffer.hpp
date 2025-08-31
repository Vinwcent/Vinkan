#ifndef VINKAN_BUFFER_HPP
#define VINKAN_BUFFER_HPP

#include <vulkan/vulkan.h>

#include "vinkan/generics/ptr_handle_wrapper.hpp"
#include "vinkan/structs/sharing_mode.hpp"

namespace vinkan {

struct BufferInfo {
  VkDeviceSize instanceSize;
  uint32_t instanceCount;
  VkBufferUsageFlags usageFlags;
  SharingMode sharingMode;
  VkMemoryPropertyFlags memoryPropertyFlags;
  VkDeviceSize minOffsetAlignment = 1;
};

class Buffer : public PtrHandleWrapper<VkBuffer> {
 public:
  Buffer(VkDevice device,
         VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
         BufferInfo bufferInfo);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE,
                     VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE,
                                        VkDeviceSize offset = 0);

  void readBuffer(void* data);

  VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE,
                      VkDeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  VkResult flushIndex(int index);
  VkDescriptorBufferInfo descriptorInfoForIndex(int index);
  VkResult invalidateIndex(int index);

  void* getMappedMemory() const { return mapped; }
  uint32_t getInstanceCount() const { return instanceCount; }
  VkDeviceSize getInstanceSize() const { return instanceSize; }

  VkDeviceSize getAlignmentSize() const { return instanceSize; }
  VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const {
    return memoryPropertyFlags;
  }
  VkDeviceSize getBufferSize() const { return bufferSize; }

 private:
  static VkDeviceSize getAlignment(VkDeviceSize instanceSize,
                                   VkDeviceSize minOffsetAlignment);
  VkDevice device_;

  void* mapped = nullptr;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;

  VkDeviceSize bufferSize;
  uint32_t instanceCount;
  VkDeviceSize instanceSize;
  VkDeviceSize alignmentSize;
  VkBufferUsageFlags usageFlags;
  VkMemoryPropertyFlags memoryPropertyFlags;
};

}  // namespace vinkan

#endif
