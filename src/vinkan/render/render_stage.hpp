#ifndef VINKAN_RENDER_STAGE_HPP
#define VINKAN_RENDER_STAGE_HPP

#include "vinkan/generics/concepts.hpp"
#include "vinkan/wrappers/render_pass.hpp"

namespace vinkan {

class RenderStage {
 public:
  template <EnumType AttachmentT>
  class Builder;

  ~RenderStage() {
    for (VkFramebuffer framebuffer : framebuffers_) {
      if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device_, framebuffer, nullptr);
      }
    }
  }

  void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
    assert(frameIndex < framebuffers_.size());
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;

    renderPassInfo.framebuffer = framebuffers_[frameIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = imageExtent_;

    std::vector<VkClearValue> clearValues(nAttachments_);
    for (auto n = 0; n < nAttachments_; ++n) {
      clearValues[n].color = {0., 0., 0., 1.0f};
    }
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(imageExtent_.width);
    viewport.height = static_cast<float>(imageExtent_.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    VkRect2D scissor{{0, 0}, imageExtent_};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

 private:
  RenderStage(VkDevice device, std::vector<VkFramebuffer> framebuffers,
              VkRenderPass renderPass, VkExtent2D imageExtent,
              uint32_t nAttachments)
      : framebuffers_(framebuffers),
        renderPass_(renderPass),
        imageExtent_(imageExtent),
        nAttachments_(nAttachments),
        device_(device) {}

  uint32_t nAttachments_;
  std::vector<VkFramebuffer> framebuffers_;
  VkRenderPass renderPass_;
  VkExtent2D imageExtent_;
  VkDevice device_;

  template <EnumType AttachmentT>
  friend class Builder;
};

template <EnumType AttachmentT>
class RenderStage::Builder {
 public:
  Builder(RenderPass<AttachmentT> &renderPass, VkDevice device,
          uint32_t nFrames)
      : device_(device), renderPass_(renderPass), nFrames_(nFrames) {}

  Builder &defineAttachment(AttachmentT attachmentIdentifier,
                            std::vector<VkImageView> attachment) {
    assert(!allAttachments_.contains(attachmentIdentifier) &&
           "Attachment already defined");
    allAttachments_.emplace(attachmentIdentifier, attachment);
    return *this;
  }

  std::unique_ptr<RenderStage> build(VkExtent2D imageExtent) {
    auto attachmentIndices = renderPass_.getAttachmentIndices();
    assert(attachmentIndices.size() == allAttachments_.size() &&
           "Could not create the RenderStage, some attachments are missing");

    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(nFrames_);
    for (int n = 0; n < nFrames_; ++n) {
      std::map<uint32_t, VkImageView> attachmentMapping{};

      for (auto &[attachmentIdentifier, index] : attachmentIndices) {
        assert(allAttachments_.contains(attachmentIdentifier));
        attachmentMapping[index] = allAttachments_[attachmentIdentifier][n];
      }
      std::vector<VkImageView> attachments;
      for (int i = 0; i < attachmentMapping.size(); ++i) {
        attachments.push_back(attachmentMapping[i]);
      }

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass_.getHandle();
      framebufferInfo.attachmentCount =
          static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = imageExtent.width;
      framebufferInfo.height = imageExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr,
                              &framebuffers[n]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }

    std::unique_ptr<RenderStage> renderStage(
        new RenderStage(device_, framebuffers, renderPass_.getHandle(),
                        imageExtent, allAttachments_.size()));
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Render stage created");
    return std::move(renderStage);
  }

 private:
  VkDevice device_;
  RenderPass<AttachmentT> &renderPass_;
  std::map<AttachmentT, std::vector<VkImageView>> allAttachments_;
  uint32_t nFrames_;
};

}  // namespace vinkan
#endif
