#ifndef VINKAN_EXTENSIONS_HPP
#define VINKAN_EXTENSIONS_HPP

#include <vulkan/vulkan.h>

#include <set>
#ifdef __APPLE__
#include <TargetConditionals.h>
#ifdef TARGET_CPU_ARM64
constexpr bool NEED_PORTABILITY_EXTENSIONS = true;
const std::set<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
#else
constexpr bool NEED_PORTABILITY_EXTENSIONS = false;
const std::set<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif
#else
constexpr bool NEED_PORTABILITY_EXTENSIONS = false;
const std::set<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif
#endif
