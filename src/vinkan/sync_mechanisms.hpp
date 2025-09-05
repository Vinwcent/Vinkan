#ifndef VINKAN_SYNC_MECHANISMS_HPP
#define VINKAN_SYNC_MECHANISMS_HPP

#include <vulkan/vulkan.h>

#include <map>
#include <vector>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/logging/logger.hpp"

namespace vinkan {

template <EnumType FenceT, EnumType SemT>
class SyncMechanisms {
 public:
  VkFence getFence(FenceT fenceIdentifier) {
    assert(fences_.contains(fenceIdentifier));
    return fences_.at(fenceIdentifier);
  }

  VkSemaphore getSemaphore(SemT semaphoreIdentifier) {
    assert(semaphores_.contains(semaphoreIdentifier));
    return semaphores_.at(semaphoreIdentifier);
  }

  SyncMechanisms(VkDevice device) : device_(device) {}

  ~SyncMechanisms() {
    for (auto& [identifier, fence] : fences_) {
      vkDestroyFence(device_, fence, nullptr);
    }
    for (auto& [identifier, semaphore] : semaphores_) {
      vkDestroySemaphore(device_, semaphore, nullptr);
    }
  }

  SyncMechanisms(const SyncMechanisms&) = delete;
  SyncMechanisms& operator=(const SyncMechanisms&) = delete;

  // Fence
  void createFence(std::vector<FenceT> fenceIdentifiers,
                   bool signaled = false) {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    for (auto fenceIdentifier : fenceIdentifiers) {
      VkFence fence;
      if (vkCreateFence(device_, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence");
      }
      fences_[fenceIdentifier] = fence;
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Long lived fences created");
  }

  void createFence(FenceT fenceIdentifier, bool signaled = false) {
    createFence(std::vector<FenceT>{fenceIdentifier}, signaled);
  }

  // Semaphore
  void createSemaphore(std::vector<SemT> semaphoreIdentifiers) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto semaphoreIdentifier : semaphoreIdentifiers) {
      VkSemaphore semaphore;
      if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &semaphore) !=
          VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphore");
      }
      semaphores_[semaphoreIdentifier] = semaphore;
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Long lived semaphores created");
  }

  void createSemaphore(SemT semaphoreIdentifier) {
    createSemaphore(std::vector<SemT>{semaphoreIdentifier});
  }

  // Free fences
  void freeFence(std::vector<FenceT> fenceIdentifiers) {
    for (auto fenceIdentifier : fenceIdentifiers) {
      assert(fences_.contains(fenceIdentifier));

      vkDestroyFence(device_, fences_[fenceIdentifier], nullptr);
      fences_.erase(fenceIdentifier);
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Fences freed");
  }

  void freeFence(FenceT fenceIdentifier) {
    freeFence(std::vector<FenceT>{fenceIdentifier});
  }

  void freeSemaphore(std::vector<SemT> semaphoreIdentifiers) {
    for (auto semaphoreIdentifier : semaphoreIdentifiers) {
      assert(semaphores_.contains(semaphoreIdentifier));

      vkDestroySemaphore(device_, semaphores_[semaphoreIdentifier], nullptr);
      semaphores_.erase(semaphoreIdentifier);
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Semaphores freed");
  }

  void freeSemaphore(SemT semaphoreIdentifier) {
    freeSemaphore(std::vector<SemT>{semaphoreIdentifier});
  }

 private:
  VkDevice device_;
  std::map<FenceT, VkFence> fences_;
  std::map<SemT, VkSemaphore> semaphores_;
};

}  // namespace vinkan
#endif
