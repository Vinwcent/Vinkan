#ifndef VINKAN_MODEL_BASE_HPP
#define VINKAN_MODEL_BASE_HPP

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "vinkan/wrappers/buffer.hpp"

namespace vinkan {

template <typename Vertex>
struct ModelData {
  std::vector<Vertex> vertices{};
  std::vector<uint32_t> indices{};
};

template <typename Vertex>
class ModelBase {
 public:
  ModelBase(VkDevice device,
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties)
      : device_(device), deviceMemoryProperties_(deviceMemoryProperties) {}

  virtual ~ModelBase() = default;

  ModelBase(const ModelBase&) = delete;
  ModelBase& operator=(const ModelBase&) = delete;

  virtual void draw(VkCommandBuffer commandBuffer) = 0;

  void transferModelToDevice(VkCommandBuffer commandBuffer,
                             const ModelData<Vertex>& modelData,
                             VkQueue transferQueue) {
    if (!modelData.vertices.empty()) {
      vertexBuffer_ =
          createVertexBuffer_(commandBuffer, modelData.vertices, transferQueue);
    }
    if (!modelData.indices.empty()) {
      indexBuffer_ =
          createIndexBuffer_(commandBuffer, modelData.indices, transferQueue);
    }
  }

 protected:
  VkDevice device_;
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties_;

  std::unique_ptr<Buffer> indexBuffer_;
  std::unique_ptr<Buffer> vertexBuffer_;

  std::unique_ptr<Buffer> createVertexBuffer_(
      VkCommandBuffer commandBuffer, const std::vector<Vertex>& vertices,
      VkQueue transferQueue) {
    auto vertexCount = static_cast<uint32_t>(vertices.size());
    if (vertexCount == 0) {
      return nullptr;
    }

    auto vertexSize = sizeof(vertices[0]);

    // Create staging buffer
    BufferInfo stagingBufferInfo{
        .instanceSize = vertexSize,
        .instanceCount = vertexCount,
        .usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
        .memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    Buffer stagingBuffer(device_, deviceMemoryProperties_, stagingBufferInfo);
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)vertices.data());
    stagingBuffer.unmap();

    // Create vertex buffer
    BufferInfo vertexBufferInfo{
        .instanceSize = vertexSize,
        .instanceCount = vertexCount,
        .usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
        .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    auto vertexBuffer = std::make_unique<Buffer>(
        device_, deviceMemoryProperties_, vertexBufferInfo);

    // Copy with sync
    copyBufferSync_(commandBuffer, stagingBuffer.getHandle(),
                    vertexBuffer->getHandle(), vertexBuffer->getBufferSize(),
                    transferQueue);

    return vertexBuffer;
  }

  std::unique_ptr<Buffer> createIndexBuffer_(
      VkCommandBuffer commandBuffer, const std::vector<uint32_t>& indices,
      VkQueue transferQueue) {
    auto indexCount = static_cast<uint32_t>(indices.size());
    if (indexCount == 0) {
      return nullptr;
    }

    auto indexSize = sizeof(indices[0]);

    // Create staging buffer
    BufferInfo stagingBufferInfo{
        .instanceSize = indexSize,
        .instanceCount = indexCount,
        .usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
        .memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    Buffer stagingBuffer(device_, deviceMemoryProperties_, stagingBufferInfo);
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());
    stagingBuffer.unmap();

    // Create index buffer
    BufferInfo indexBufferInfo{
        .instanceSize = indexSize,
        .instanceCount = indexCount,
        .usageFlags =
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = {.value = VK_SHARING_MODE_EXCLUSIVE},
        .memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    auto indexBuffer = std::make_unique<Buffer>(
        device_, deviceMemoryProperties_, indexBufferInfo);

    // Copy with sync
    copyBufferSync_(commandBuffer, stagingBuffer.getHandle(),
                    indexBuffer->getHandle(), indexBuffer->getBufferSize(),
                    transferQueue);

    return indexBuffer;
  }

  void bindBuffers_(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer,
                    VkBuffer indexBuffer) {
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }

 private:
  void copyBufferSync_(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                       VkBuffer dstBuffer, VkDeviceSize size,
                       VkQueue transferQueue) {
    // Begin recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer");
    }

    // Record copy command
    VkBufferCopy copyDesc{};
    copyDesc.dstOffset = 0;
    copyDesc.srcOffset = 0;
    copyDesc.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyDesc);

    // End recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffer");
    }

    // Submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to submit command buffer");
    }

    // Wait for completion
    vkQueueWaitIdle(transferQueue);

    // Reset command buffer for next use
    if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
      throw std::runtime_error("Failed to reset command buffer");
    }
  }
};

}  // namespace vinkan

#endif  // VINKAN_MODEL_BASE_HPP
