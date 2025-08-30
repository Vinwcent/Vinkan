#include "glfw_vk_surface.hpp"

#include <cassert>
#include <stdexcept>

namespace vinkan {

GlfwVkSurface::GlfwVkSurface(GLFWwindow *window, VkInstance instance) {
  if (glfwCreateWindowSurface(instance, window, nullptr, &handle_) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "Could not initialize the vulkan surface through glfw");
  }
  surfaceInstance_ = instance;
}

GlfwVkSurface::~GlfwVkSurface() {
  if (!isHandleValid()) {
    return;
  }
  vkDestroySurfaceKHR(surfaceInstance_, handle_, nullptr);

}  // namespace vvw_vk

}  // namespace vinkan
