#include "instance.hpp"

#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <unordered_set>

#include "vinkan/logging/logger.hpp"

/****************************************
 *  UTILS FUNCTION FOR DEBUG MESSENGER  *
 ****************************************/

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  std::cerr << "Validation layer:\n" << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

/**************
 *  INSTANCE  *
 **************/

namespace vvw_vk {
Instance::Instance(InstanceInfo &instanceInfo) {
  // Create app info
  VkApplicationInfo appInfo{};
  populateAppInfo_(instanceInfo, appInfo);

  // Create the instance extensions
  std::vector<const char *> extensions{};
  populateExtensions_(instanceInfo, extensions);

  VkInstanceCreateInfo createInfo{};
  prepareCreateInfo_(instanceInfo, createInfo);
  // Add the app info
  createInfo.pApplicationInfo = &appInfo;

  // Add the extensions
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // Add the validation layers
  populateValidationLayers_(instanceInfo, createInfo);

  if (vkCreateInstance(&createInfo, nullptr, &handle_) != VK_SUCCESS) {
    throw std::runtime_error("Could not create the vulkan instance");
  }
  SPDLOG_LOGGER_INFO(get_vinkan_logger(), "VkInstance created");

  if (instanceInfo.validationLayers.size() > 0) {
    // If we have validation layers, once the instance is created, we had the
    // debug messenger
    setupDebugMessenger_();
  }
}

Instance::~Instance() {
  if (!isHandleValid()) {
    return;
  }

  if (debugMessenger_ != nullptr) {
    destroyDebugUtilsMessengerEXT(handle_, debugMessenger_, nullptr);
  }
  vkDestroyInstance(handle_, nullptr);
}

void Instance::populateValidationLayers_(InstanceInfo &instanceInfo,
                                         VkInstanceCreateInfo &createInfo) {
  if (instanceInfo.validationLayers.size() > 0) {
    // Add the validation layers
    if (!areValidationLayersAvailable_(instanceInfo.validationLayers)) {
      throw std::runtime_error("Validation layers asked are not available");
    }

    createInfo.enabledLayerCount =
        static_cast<uint32_t>(instanceInfo.validationLayers.size());
    createInfo.ppEnabledLayerNames = instanceInfo.validationLayers.data();
  }
}

void Instance::prepareCreateInfo_(InstanceInfo &instanceInfo,
                                  VkInstanceCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  // If we have the portability, the instance must report them too
  if (instanceInfo.includePortabilityExtensions) {
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
  createInfo.enabledLayerCount = 0;
  createInfo.pNext = nullptr;
}

void Instance::populateAppInfo_(InstanceInfo &instanceInfo,
                                VkApplicationInfo &appInfo) {
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.applicationVersion = VK_MAKE_VERSION(instanceInfo.appVersion.major,
                                               instanceInfo.appVersion.minor,
                                               instanceInfo.appVersion.patch);
  appInfo.apiVersion = instanceInfo.apiVersion;
  appInfo.pApplicationName = instanceInfo.appName.data();
  appInfo.engineVersion = VK_MAKE_VERSION(instanceInfo.engineVersion.major,
                                          instanceInfo.engineVersion.minor,
                                          instanceInfo.engineVersion.patch);
  appInfo.pEngineName = instanceInfo.engineName.data();
}

void Instance::populateExtensions_(InstanceInfo &instanceInfo,
                                   std::vector<const char *> &extensions) {
  extensions = instanceInfo.extraVkExtensions;
  if (instanceInfo.validationLayers.size() > 0) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  if (instanceInfo.includePortabilityExtensions) {
    extensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  }

  if (!areExtensionsAvailables_(extensions)) {
    throw std::runtime_error(
        "You don't have the required extensions in your vulkan "
        "installation");
  }
}

bool Instance::areExtensionsAvailables_(
    const std::vector<const char *> &extensionsNames) {
  // Get the properties of available extensions
  uint32_t availExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &availExtensionCount,
                                         nullptr);
  std::vector<VkExtensionProperties> availExtensionsProps(availExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &availExtensionCount,
                                         availExtensionsProps.data());

  // Get the set of name of available extensions
  std::unordered_set<std::string> availExtensionsName;
  for (const auto &availExtensionProps : availExtensionsProps) {
    availExtensionsName.insert(availExtensionProps.extensionName);
  }

  // Check for each extension if it is in the available extensions
  for (const auto &extensionName : extensionsNames) {
    if (availExtensionsName.find(extensionName) == availExtensionsName.end()) {
      return false;
    }
  }

  return true;
}

bool Instance::areValidationLayersAvailable_(
    const std::vector<const char *> &layers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : layers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

void Instance::populateDebugMessengerCreateInfo_(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;  // Optional
}

void Instance::setupDebugMessenger_() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo_(createInfo);
  if (createDebugUtilsMessengerEXT(handle_, &createInfo, nullptr,
                                   &debugMessenger_) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

}  // namespace vvw_vk

