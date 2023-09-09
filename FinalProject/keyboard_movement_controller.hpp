#pragma once

#include "lve_game_object.hpp"
#include "lve_window.hpp"

namespace lve {
class KeyboardMovementController {
private:
    const int MAX_VOXEL_COOLDOWN = 1000;
    const int MOUSE_TOGGLE_COOLDOWN = 3000;
    int mouseToggleCooldown;
    int placeVoxelCooldown = 0;
 public:
  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_SPACE;
    int moveDown = GLFW_KEY_LEFT_SHIFT;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_UP;
    int lookDown = GLFW_KEY_DOWN;
    int placeVoxel = GLFW_KEY_E;
    int removeVoxel = GLFW_KEY_Q;
    int updateColors = GLFW_KEY_F1;
    int resetCameraPos = GLFW_KEY_LEFT_CONTROL;
    int importModel = GLFW_KEY_I;
    int unlockMouse = GLFW_KEY_ESCAPE;
  };

  void moveInPlaneXZ(GLFWwindow* window, float dt, LveGameObject& gameObject);

  bool placeVoxelCheck(GLFWwindow* window) {
      if (placeVoxelCooldown == 0 && glfwGetKey(window, keys.placeVoxel) == GLFW_PRESS) {
          placeVoxelCooldown = MAX_VOXEL_COOLDOWN;
          return true;
      }
      if (placeVoxelCooldown > 0) placeVoxelCooldown--;
      return false;
  }

  bool removeVoxelCheck(GLFWwindow* window) {
      if (placeVoxelCooldown == 0 && glfwGetKey(window, keys.removeVoxel) == GLFW_PRESS) {
          placeVoxelCooldown = MAX_VOXEL_COOLDOWN;
          return true;
      }
      if (placeVoxelCooldown > 0) placeVoxelCooldown--;
      return false;
  }

  bool resetCameraCheck(GLFWwindow* window) {
      return (glfwGetKey(window, keys.resetCameraPos) == GLFW_PRESS);
  }

  bool updateColorCheck(GLFWwindow* window) {
      return (glfwGetKey(window, keys.updateColors) == GLFW_PRESS);
  }

  bool importModelCheck(GLFWwindow* window) {
      return (glfwGetKey(window, keys.importModel) == GLFW_PRESS);
  }

  bool toggleMouseLock(GLFWwindow* window) {
      if (mouseToggleCooldown == 0) {
          mouseToggleCooldown = MOUSE_TOGGLE_COOLDOWN;
          return (glfwGetKey(window, keys.unlockMouse) == GLFW_PRESS);
      }
      if (mouseToggleCooldown > 0) mouseToggleCooldown--;
      return false;
  }

  KeyMappings keys{};
  float moveSpeed{3.f};
  float lookSpeed{1.5f};
};
}  // namespace lve