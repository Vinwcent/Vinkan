#ifndef VINKAN_MODEL_HPP
#define VINKAN_MODEL_HPP

#include "model_base.hpp"

namespace vinkan {

template <typename Vertex>
class Model : public ModelBase<Vertex> {
 public:
  Model(VkDevice device,
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties,
        const ModelData<Vertex>& modelData)
      : ModelBase<Vertex>(device, deviceMemoryProperties),
        vertexCount_(static_cast<uint32_t>(modelData.vertices.size())),
        indexCount_(static_cast<uint32_t>(modelData.indices.size())) {}

  void draw(VkCommandBuffer commandBuffer) override {
    if (this->vertexBuffer_) {
      if (this->indexBuffer_) {
        // Draw with indices
        this->bindBuffers_(commandBuffer, this->vertexBuffer_->getHandle(),
                           this->indexBuffer_->getHandle());
        vkCmdDrawIndexed(commandBuffer, indexCount_, 1, 0, 0, 0);
      } else {
        // Draw without indices
        VkBuffer buffers[] = {this->vertexBuffer_->getHandle()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdDraw(commandBuffer, vertexCount_, 1, 0, 0);
      }
    }
  }

  uint32_t getVertexCount() const { return vertexCount_; }
  uint32_t getIndexCount() const { return indexCount_; }

 private:
  uint32_t vertexCount_;
  uint32_t indexCount_;
};

}  // namespace vinkan

#endif  // VINKAN_MODEL_HPP
