#include "physical_device.hpp"

#include <cstdint>
#include <set>
#include <stdexcept>

#include "vinkan/logging/logger.hpp"

void logQueues(std::vector<vinkan::QueueFamilyInfo> queuesInfo,
               VkSurfaceKHR surface) {
  std::ostringstream log;
  log << std::to_string(queuesInfo.size()) + " queue families found";
  for (auto queueInfo : queuesInfo) {
    log << "\nQueue family " << std::to_string(queueInfo.queueIndex) << " has "
        << std::to_string(queueInfo.queueCount) << " queues.";
    bool supportSomething = false;
    if (queueInfo.supportCompute()) {
      supportSomething = true;
      log << "\n> This queue family support compute";
    }
    if (queueInfo.supportGfx()) {
      supportSomething = true;
      log << "\n> This queue family support graphics";
    }
    if (surface != nullptr && queueInfo.supportPresentation(surface)) {
      supportSomething = true;
      log << "\n> This queue family support the surface";
    }
    if (!supportSomething) {
      log << "\n> This queue doesn't support compute, gfx or presentation";
    }
  }
  SPDLOG_LOGGER_INFO(get_vinkan_logger(), log.str());
}

namespace vinkan {

PhysicalDevice::PhysicalDevice(PhysicalDeviceInfo physicalDeviceInfo,
                               VkInstance instance) {
  if (physicalDeviceInfo.surfaceSupportRequested.has_value()) {
    withSurfaceSupport = true;
    surface_ = physicalDeviceInfo.surfaceSupportRequested.value();
  }
  auto devices = getVkCompatibleDevice_(instance);
  for (auto physicalDevice : devices) {
    if (isSuitable_(physicalDevice, physicalDeviceInfo)) {
      VkPhysicalDeviceProperties physicalDeviceProperties;
      vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
      SPDLOG_LOGGER_INFO(get_vinkan_logger(),
                         "Physical device selected " +
                             std::string(physicalDeviceProperties.deviceName));
      handle_ = physicalDevice;
      logQueues(getQueues(), surface_);
      return;
    }
  }
  throw std::runtime_error("Could not find a suitable physical device");
}

std::vector<QueueFamilyInfo> PhysicalDevice::getQueues() {
  return getQueuesInfo_(handle_);
}

bool PhysicalDevice::isSuitable_(VkPhysicalDevice physicalDevice,
                                 PhysicalDeviceInfo physicalDeviceInfo) const {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  // Check extensions
  if (!supportExtensions_(physicalDevice, physicalDeviceInfo.extensions)) {
    return false;
  }
  // Get the queues
  auto queuesInfo = getQueuesInfo_(physicalDevice);
  for (auto requestedQueueFlag : physicalDeviceInfo.requestedQueueFlags) {
    bool support = false;
    for (auto queueInfo : queuesInfo) {
      if (queueInfo.supportQueueFlags(requestedQueueFlag)) {
        support = true;
        break;
      }
    }
    if (!support) {
      return false;
    }
  }

  // Check surface support if needed
  if (withSurfaceSupport) {
    VkSurfaceKHR surface = physicalDeviceInfo.surfaceSupportRequested.value();
    auto surfaceSupportDetails = getSurfaceSupportDetails_(physicalDevice);
    bool isSurfaceCompatible = !surfaceSupportDetails.formats.empty() &&
                               !surfaceSupportDetails.presentModes.empty();
    if (!isSurfaceCompatible) {
      return false;
    }
    // Check at least one queue support the surface
    bool surfaceSupport = false;
    for (auto queueInfo : queuesInfo) {
      if (queueInfo.supportPresentation(surface_)) {
        surfaceSupport = true;
        break;
      }
    }
    if (!surfaceSupport) {
      return false;
    }
  }
  return true;
}

SurfaceSupportDetails PhysicalDevice::getSurfaceSupportDetails() {
  return getSurfaceSupportDetails_(handle_);
}
SurfaceSupportDetails PhysicalDevice::getSurfaceSupportDetails_(
    VkPhysicalDevice physicalDevice) const {
  SurfaceSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface_,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount,
                                       nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_,
                                            &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_,
                                              &presentModeCount,
                                              details.presentModes.data());
  }
  return details;
}

SurfaceSupportDetails getSurfaceSupportDetails(VkPhysicalDevice physicalDevice,
                                               VkSurfaceKHR surface);

std::vector<VkPhysicalDevice> PhysicalDevice::getVkCompatibleDevice_(
    VkInstance instance) {
  uint32_t nPhyDevices = 0;
  vkEnumeratePhysicalDevices(instance, &nPhyDevices, nullptr);
  if (nPhyDevices == 0) {
    throw std::runtime_error("No devices with Vulkan Support was found");
  }

  std::vector<VkPhysicalDevice> physicalDevices(nPhyDevices);
  vkEnumeratePhysicalDevices(instance, &nPhyDevices, physicalDevices.data());
  SPDLOG_LOGGER_INFO(
      get_vinkan_logger(),
      std::to_string(nPhyDevices) + " Vulkan compatible devices found");
  return physicalDevices;
}

std::vector<QueueFamilyInfo> PhysicalDevice::getQueuesInfo_(
    VkPhysicalDevice physicalDevice) const {
  uint32_t nQueueFamilies = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nQueueFamilies,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(nQueueFamilies);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &nQueueFamilies,
                                           queueFamilies.data());
  std::vector<QueueFamilyInfo> queuesInfo{};

  for (int i = 0; i < queueFamilies.size(); ++i) {
    auto &queueFamily = queueFamilies[i];
    if (queueFamily.queueCount == 0) {
      continue;
    }
    QueueFamilyInfo queueInfo(static_cast<uint32_t>(i), queueFamily.queueCount,
                              queueFamily.queueFlags, physicalDevice);
    queuesInfo.push_back(queueInfo);
  }
  return queuesInfo;
}

bool PhysicalDevice::supportExtensions_(
    VkPhysicalDevice physicalDevice,
    const std::set<const char *> &extensions) const {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(extensions.begin(),
                                           extensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

}  // namespace vinkan

