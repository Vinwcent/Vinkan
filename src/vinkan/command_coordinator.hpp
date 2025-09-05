#ifndef VINKAN_COMMAND_COORDINATOR_HPP
#define VINKAN_COMMAND_COORDINATOR_HPP

#include <vulkan/vulkan.h>

#include <map>
#include <set>
#include <vector>
#include <vinkan/logging/logger.hpp>

#include "vinkan/generics/concepts.hpp"

namespace vinkan {

struct SubmitCommandBufferInfo {
  std::vector<VkSemaphore> waitSemaphores{};
  std::vector<VkPipelineStageFlags> waitDstStages{};
  std::vector<VkSemaphore> signalSemaphores{};
  VkFence signalFence = VK_NULL_HANDLE;
  VkQueue queue;
};

template <EnumType CommandT, EnumType CommandPoolT>
class CommandCoordinator {
 public:
  VkCommandBuffer get(CommandT commandIdentifier) {
    assert(commandBuffers_.contains(commandIdentifier));
    return commandBuffers_.at(commandIdentifier);
  }

  CommandCoordinator(VkDevice device) : device_(device) {}
  ~CommandCoordinator() {
    for (auto& [identifier, pool] : commandPools_) {
      vkDestroyCommandPool(device_, pool, nullptr);
    }
  }

  CommandCoordinator(const CommandCoordinator&) = delete;
  CommandCoordinator& operator=(const CommandCoordinator&) = delete;

  void createCommandPool(CommandPoolT commandPoolIdentifier,
                         uint32_t queueFamilyIndex, bool singleUsagePool) {
    assert(!commandPools_.contains(commandPoolIdentifier));
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = singleUsagePool
                         ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
                         : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool pool;
    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create command pool");
    }
    if (singleUsagePool) {
      singleUsePools_.insert(commandPoolIdentifier);
    }
    commandPools_[commandPoolIdentifier] = pool;
    SPDLOG_LOGGER_TRACE(get_vinkan_logger(), "Command pool created");
  }
  void resetCommandBuffer(CommandT commandIdentifier) {
    assert(commandBuffers_.contains(commandIdentifier));

    if (vkResetCommandBuffer(commandBuffers_[commandIdentifier], 0) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to reset command buffer");
    }
    SPDLOG_LOGGER_TRACE(get_vinkan_logger(), "Command buffer reset");
  }

  void createLongLivedCommand(std::vector<CommandT> commandIdentifiers,
                              CommandPoolT commandPoolIdentifier) {
    assert(commandPools_.contains(commandPoolIdentifier));
    auto commandPool = commandPools_[commandPoolIdentifier];

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(commandIdentifiers.size());

    std::vector<VkCommandBuffer> commandBuffers(commandIdentifiers.size());
    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate command buffers");
    }

    for (size_t i = 0; i < commandIdentifiers.size(); ++i) {
      commandBuffers_[commandIdentifiers[i]] = commandBuffers[i];
      commandToPool_[commandIdentifiers[i]] = commandPool;
    }
    SPDLOG_LOGGER_TRACE(get_vinkan_logger(),
                        "Long lived command buffers created");
  }

  void createLongLivedCommand(CommandT commandIdentifier,
                              CommandPoolT commandPoolIdentifier) {
    createLongLivedCommand(std::vector<CommandT>{commandIdentifier},
                           commandPoolIdentifier);
  }

  VkCommandBuffer createSingleUseCommandBuffer(
      CommandPoolT commandPoolIdentifier) {
    assert(commandPools_.contains(commandPoolIdentifier));
    assert(singleUsePools_.contains(commandPoolIdentifier));
    auto commandPool = commandPools_[commandPoolIdentifier];

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate single use command buffer");
    }

    SPDLOG_LOGGER_TRACE(get_vinkan_logger(),
                        "Single use command buffer created");
    return commandBuffer;
  }

  void freeCommandBuffer(CommandT commandIdentifier) {
    assert(commandBuffers_.contains(commandIdentifier));
    assert(commandToPool_.contains(commandIdentifier));

    auto commandPool = commandToPool_[commandIdentifier];
    VkCommandBuffer commandBuffer = commandBuffers_[commandIdentifier];
    freeCommandBuffer(commandPool, commandBuffer);
    commandBuffers_.erase(commandIdentifier);
    commandToPool_.erase(commandIdentifier);
  }

  void freeCommandBuffer(CommandPoolT commandPoolIdentifier,
                         VkCommandBuffer commandBuffer) {
    assert(commandPools_.contains(commandPoolIdentifier));
    auto commandPool = commandPools_[commandPoolIdentifier];

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
    SPDLOG_LOGGER_TRACE(get_vinkan_logger(), "Command buffer freed");
  }

  void beginCommandBuffer(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer");
    }
  }

  VkCommandBuffer beginCommandBuffer(CommandT commandIdentifier) {
    assert(commandBuffers_.contains(commandIdentifier));
    VkCommandBuffer commandBuffer = commandBuffers_[commandIdentifier];
    beginCommandBuffer(commandBuffer);
    return commandBuffer;
  }

  void endCommandBuffer(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
  }

  void submitCommandBuffer(std::vector<VkCommandBuffer> commandBuffers,
                           SubmitCommandBufferInfo submitBufferInfo) {
    assert(submitBufferInfo.waitDstStages.size() ==
           submitBufferInfo.waitSemaphores.size());
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount =
        static_cast<uint32_t>(submitBufferInfo.waitSemaphores.size());
    submitInfo.pWaitSemaphores = submitBufferInfo.waitSemaphores.data();
    submitInfo.pWaitDstStageMask = submitBufferInfo.waitDstStages.data();
    submitInfo.commandBufferCount =
        static_cast<uint32_t>(commandBuffers.size());
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount =
        static_cast<uint32_t>(submitBufferInfo.signalSemaphores.size());
    submitInfo.pSignalSemaphores = submitBufferInfo.signalSemaphores.data();

    if (vkQueueSubmit(submitBufferInfo.queue, 1, &submitInfo,
                      submitBufferInfo.signalFence) != VK_SUCCESS) {
      throw std::runtime_error("Failed to submit command buffer");
    }
    SPDLOG_LOGGER_TRACE(get_vinkan_logger(), "Command buffer submitted");
  }

  void submitCommandBuffer(VkCommandBuffer commandBuffer,
                           SubmitCommandBufferInfo submitBufferInfo) {
    submitCommandBuffer(std::vector<VkCommandBuffer>{commandBuffer},
                        submitBufferInfo);
  }

 private:
  VkDevice device_;

  std::set<CommandPoolT> singleUsePools_;
  std::map<CommandPoolT, VkCommandPool> commandPools_;
  std::map<CommandT, VkCommandBuffer> commandBuffers_;
  std::map<CommandT, VkCommandPool> commandToPool_;
};

}  // namespace vinkan
#endif
