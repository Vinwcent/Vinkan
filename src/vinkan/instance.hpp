#ifndef VVW_INSTANCE_HPP
#define VVW_INSTANCE_HPP value

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "vinkan/generics/ptr_handle_wrapper.hpp"

struct SemanticVersion {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
};

struct InstanceInfo {
  std::string appName;
  SemanticVersion appVersion;
  std::string engineName;
  SemanticVersion engineVersion;
  uint32_t apiVersion;

  std::vector<const char *> validationLayers;
  bool includePortabilityExtensions;  // Needed to identify M-Series Mac GPU
  std::vector<const char *> &extraVkExtensions;
};

namespace vvw_vk {
class Instance : public PtrHandleWrapper<VkInstance> {
 public:
  Instance(InstanceInfo &instanceInfo);
  ~Instance();

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;

 private:
  VkDebugUtilsMessengerEXT debugMessenger_;

  // Populate
  void prepareCreateInfo_(InstanceInfo &instanceInfo,
                          VkInstanceCreateInfo &instanceCreateInfo);
  void populateAppInfo_(InstanceInfo &instanceInfo, VkApplicationInfo &appInfo);
  void populateExtensions_(InstanceInfo &instanceInfo,
                           std::vector<const char *> &extensions);
  void populateValidationLayers_(InstanceInfo &instanceInfo,
                                 VkInstanceCreateInfo &createInfo);
  void populateDebugMessengerCreateInfo_(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  // Check
  bool areExtensionsAvailables_(
      const std::vector<const char *> &extensionsNames);
  bool areValidationLayersAvailable_(const std::vector<const char *> &layers);

  // Debug messenger
  void setupDebugMessenger_();
};

}  // namespace vvw_vk

#endif /* ifndef VVW_INSTANCE_HPP */

