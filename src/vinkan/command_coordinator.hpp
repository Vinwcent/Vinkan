#ifndef VINKAN_COMMAND_COORDINATOR_HPP
#define VINKAN_COMMAND_COORDINATOR_HPP

#include <vulkan/vulkan.h>

#include <map>
#include <vector>

#include "vinkan/generics/concepts.hpp"
#include "vinkan/logging/logger.hpp"

namespace vinkan {

struct SubmitCommandBufferInfo {
  std::vector<VkSemaphore> waitSemaphores{};
  std::vector<VkSemaphore> signalSemaphores{};
  VkFence signalFence = VK_NULL_HANDLE;
  VkQueue queue;
};

template <EnumType CommandT, EnumType CommandPoolT,
          EnumType SingleUseCommandPoolT>
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
    for (auto& [identifier, pool] : singleUseCommandPools_) {
      vkDestroyCommandPool(device_, pool, nullptr);
    }
  }

  CommandCoordinator(const CommandCoordinator&) = delete;
  CommandCoordinator& operator=(const CommandCoordinator&) = delete;

  void createCommandPool(CommandPoolT commandPoolIdentifier,
                         uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr,
                            &commandPools_[commandPoolIdentifier]) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create command pool");
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Command pool created");
  }

  void createCommandPool(SingleUseCommandPoolT commandPoolIdentifier,
                         uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr,
                            &singleUseCommandPools_[commandPoolIdentifier]) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create single use command pool");
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Single use command pool created");
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
    SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                       "Long lived command buffers created");
  }

  void createLongLivedCommand(CommandT commandIdentifier,
                              CommandPoolT commandPoolIdentifier) {
    createLongLivedCommand(std::vector<CommandT>{commandIdentifier},
                           commandPoolIdentifier);
  }

  void createSingleUseCommand(CommandT commandIdentifier,
                              SingleUseCommandPoolT commandPoolIdentifier) {
    assert(singleUseCommandPools_.contains(commandPoolIdentifier));
    auto commandPool = singleUseCommandPools_[commandPoolIdentifier];

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
    commandBuffers_[commandIdentifier] = commandBuffer;
    commandToPool_[commandIdentifier] = commandPool;
    SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                       "Single use command buffer created");
  }

  void createSingleUseCommand(std::vector<CommandT> commandIdentifiers,
                              SingleUseCommandPoolT commandPoolIdentifier) {
    assert(singleUseCommandPools_.contains(commandPoolIdentifier));
    auto commandPool = singleUseCommandPools_[commandPoolIdentifier];

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(commandIdentifiers.size());

    std::vector<VkCommandBuffer> commandBuffers(commandIdentifiers.size());
    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate single use command buffers");
    }

    for (size_t i = 0; i < commandIdentifiers.size(); ++i) {
      commandBuffers_[commandIdentifiers[i]] = commandBuffers[i];
      commandToPool_[commandIdentifiers[i]] = commandPool;
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                       "Single use command buffers created");
  }

  void freeCommandBuffer(std::vector<CommandT> commandIdentifiers) {
    for (auto commandIdentifier : commandIdentifiers) {
      assert(commandBuffers_.contains(commandIdentifier));
      assert(commandToPool_.contains(commandIdentifier));

      auto commandBuffer = commandBuffers_[commandIdentifier];
      auto commandPool = commandToPool_[commandIdentifier];

      vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
      commandBuffers_.erase(commandIdentifier);
      commandToPool_.erase(commandIdentifier);
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Command buffers freed");
  }

  void freeCommandBuffer(CommandT commandIdentifier) {
    freeCommandBuffer(std::vector<CommandT>{commandIdentifier});
  }

  void beginCommandBuffer(std::vector<CommandT> commandIdentifiers) {
    for (auto commandIdentifier : commandIdentifiers) {
      assert(commandBuffers_.contains(commandIdentifier));

      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = 0;
      beginInfo.pInheritanceInfo = nullptr;

      if (vkBeginCommandBuffer(commandBuffers_[commandIdentifier],
                               &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
      }
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Command buffer recording begun");
  }

  void beginCommandBuffer(CommandT commandIdentifier) {
    beginCommandBuffer(std::vector<CommandT>{commandIdentifier});
  }

  void endCommandBuffer(std::vector<CommandT> commandIdentifiers) {
    for (auto commandIdentifier : commandIdentifiers) {
      assert(commandBuffers_.contains(commandIdentifier));

      if (vkEndCommandBuffer(commandBuffers_[commandIdentifier]) !=
          VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
      }
    }
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Command buffer recording ended");
  }

  void endCommandBuffer(CommandT commandIdentifier) {
    endCommandBuffer(std::vector<CommandT>{commandIdentifier});
  }

  void submitCommandBuffer(std::vector<CommandT> commandIdentifiers,
                           SubmitCommandBufferInfo submitBufferInfo) {
    std::vector<VkCommandBuffer> commandBuffers;
    for (auto commandIdentifier : commandIdentifiers) {
      assert(commandBuffers_.contains(commandIdentifier));
      commandBuffers.push_back(commandBuffers_[commandIdentifier]);
    }

    // TODO: Make wait stages a parameter
    std::vector<VkPipelineStageFlags> waitStages(
        submitBufferInfo.waitSemaphores.size(),
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount =
        static_cast<uint32_t>(submitBufferInfo.waitSemaphores.size());
    submitInfo.pWaitSemaphores = submitBufferInfo.waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
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
    SPDLOG_LOGGER_INFO(get_vinkan_logger(), "Command buffer submitted");
  }

  void submitCommandBuffer(CommandT commandIdentifier,
                           SubmitCommandBufferInfo submitBufferInfo) {
    submitCommandBuffer(std::vector<CommandT>{commandIdentifier},
                        submitBufferInfo);
  }

 private:
  VkDevice device_;

  std::map<CommandPoolT, VkCommandPool> commandPools_;
  std::map<SingleUseCommandPoolT, VkCommandPool> singleUseCommandPools_;
  std::map<CommandT, VkCommandBuffer> commandBuffers_;
  std::map<CommandT, VkCommandPool> commandToPool_;
};

}  // namespace vinkan
#endif
