#ifndef VINKAN_RENDER_PASS_BUILDER_HPP
#define VINKAN_RENDER_PASS_BUILDER_HPP

#include <vulkan/vulkan_core.h>

#include <map>
#include <optional>
#include <stdexcept>
#include <vector>

#include "vinkan/generics/concepts.hpp"
namespace vinkan {

template <EnumType T>
class SubpassInfo {
  std::vector<T> colorAttachments;
  std::vector<T> inputAttachments;
  std::vector<T> preserveAttachments;
  std::optional<T> depthStencilAttachment;
  VkPipelineStageFlags dstStage;
  VkAccessFlags dstFlags;
};

struct InternalSubpassInfo {
  std::vector<VkAttachmentReference> colorAttachments{};
  std::vector<VkAttachmentReference> inputAttachments{};
  std::vector<uint32_t> preserveAttachments{};
  std::optional<VkAttachmentReference> depthAttachment{};
};

template <EnumType T>
class RenderPassBuilder {
 public:
  void addAttachment(T attachmentType,
                     VkAttachmentDescription attachmentDescription) {
    assert(!attachmentIndices.contains(attachmentType));
    uint32_t attachmentIndex = attachments_.size();
    attachments_.push_back(attachmentDescription);
    // TODO: Enable different layout at the subpass addition step
    attachmentRefs_.push_back(
        VkAttachmentReference{.attachment = attachmentIndex,
                              .layout = attachmentDescription.finalLayout});
    attachmentIndices[attachmentType] = attachmentIndex;
  }
  void addSubpass(SubpassInfo<T> subpassInfo) {
    auto intSubpassInfo = getInternalInfo_(subpassInfo);
    intSubpassInfo_.push_back(intSubpassInfo);
    auto &info = intSubpassInfo_[intSubpassInfo_.size() - 1];
    VkSubpassDescription subpass{
        .inputAttachmentCount = info.inputAttachments.size(),
        .pInputAttachments = info.inputAttachments.data(),
        .colorAttachmentCount = info.colorAttachments.size(),
        .pColorAttachments = info.colorAttachments.data(),
        .preserveAttachmentCount = info.preserveAttachments.size(),
        .pPreserveAttachments = info.preserveAttachments.data()};
    if (info.depthAttachment.has_value()) {
      subpass.pDepthStencilAttachment = &info.depthAttachment.value();
    }
    subpasses_.push_back(subpass);

    VkSubpassDependency dependency{};
    if (subpassDependencies_.size() == 0) {
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
      dependency.dstSubpass = subpasses_.size() - 2;
    }
    dependency.dstSubpass = subpasses_.size() - 1;
    dependency.dstStageMask = subpassInfo.dstStage;
    dependency.dstAccessMask = subpassInfo.dstFlags;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    subpassDependencies_.push_back(dependency);
  }

  VkRenderPass build(VkDevice device) {
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments_.size());
    renderPassInfo.pAttachments = attachments_.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses_.size());
    renderPassInfo.pSubpasses = subpasses_.data();
    renderPassInfo.dependencyCount =
        static_cast<uint32_t>(subpassDependencies_.size());
    renderPassInfo.pDependencies = subpassDependencies_.data();

    VkRenderPass renderPass;
    VkResult result =
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create render pass!");
    }

    return renderPass;
  }

 private:
  std::vector<InternalSubpassInfo> intSubpassInfo_{};  // For memory consistency

  std::map<T, uint32_t> attachmentIndices{};
  std::vector<VkAttachmentDescription> attachments_{};
  std::vector<VkAttachmentReference> attachmentRefs_{};

  std::vector<VkSubpassDescription> subpasses_{};
  std::vector<VkSubpassDependency> subpassDependencies_{};

  InternalSubpassInfo getInternalInfo_(SubpassInfo<T> subpassInfo) {
    InternalSubpassInfo intSubpassInfo{};
    for (auto colorAttachment : subpassInfo.colorAttachments) {
      assert(attachmentIndices.contains(colorAttachment));
      uint32_t attachmentIndex = attachmentIndices[colorAttachment];
      auto attachmentReference = attachmentRefs_[attachmentIndex];
      intSubpassInfo.colorAttachments.push_back(attachmentReference);
    }
    for (auto inputAttachment : subpassInfo.inputAttachments) {
      assert(attachmentIndices.contains(inputAttachment));
      uint32_t attachmentIndex = attachmentIndices[inputAttachment];
      auto attachmentReference = attachmentRefs_[attachmentIndex];
      intSubpassInfo.inputAttachments.push_back(attachmentReference);
    }
    for (auto preserveAttachment : subpassInfo.preserveAttachments) {
      assert(attachmentIndices.contains(preserveAttachment));
      uint32_t attachmentIndex = attachmentIndices[preserveAttachment];
      intSubpassInfo.preserveAttachments.push_back(attachmentIndex);
    }
    if (subpassInfo.depthStencilAttachment.has_value()) {
      assert(attachmentIndices.contains(
          subpassInfo.depthStencilAttachment.value()));
      uint32_t attachmentIndex =
          attachmentIndices[subpassInfo.depthStencilAttachment.value()];
      auto attachmentReference = attachmentRefs_[attachmentIndex];
      intSubpassInfo.depthAttachment = attachmentReference;
    } else {
      intSubpassInfo.depthAttachment = std::nullopt;
    }
    return intSubpassInfo;
  }
};
}  // namespace vinkan
#endif
