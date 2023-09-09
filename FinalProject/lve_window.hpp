#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

namespace lve {

class LveWindow {
 public:
  LveWindow(int w, int h, std::string name);
  ~LveWindow();

  LveWindow(const LveWindow &) = delete;
  LveWindow &operator=(const LveWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void bindToWindow() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); mouse = false; }
  void freeFromWindow() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); mouse = true; }

  void toggleMouse() {
	  if (mouse) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	  else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	  mouse = !mouse;
	  std::cout << "Mouse Toggled!\n";
  }

  void resetWindowResizedFlag() { framebufferResized = false; }
  GLFWwindow *getGLFWwindow() const { return window; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;
  bool mouse = true;

  std::string windowName;
  GLFWwindow *window;
};
}  // namespace lve
