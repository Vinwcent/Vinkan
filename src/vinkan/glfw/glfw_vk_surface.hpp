#ifndef VINKAN_GLFW_VK_SURFACE_HPP
#define VINKAN_GLFW_VK_SURFACE_HPP

#include <vulkan/vulkan_core.h>

#include "vinkan/generics/ptr_handle_wrapper.hpp"

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vinkan {

class GlfwVkSurface : public PtrHandleWrapper<VkSurfaceKHR> {
 public:
  GlfwVkSurface(GLFWwindow *window, VkInstance instance);
  ~GlfwVkSurface();

 private:
  VkInstance surfaceInstance_;
};

}  // namespace vinkan

#endif /* GLFW_SURFACE_H */

