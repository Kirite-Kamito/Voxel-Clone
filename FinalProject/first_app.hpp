#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_camera.hpp"
#include "lve_window.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();
  void importModel();
  void makeCube(LveCamera cam, glm::vec3 color);
  void removeCube(LveCamera cam);

  void cull(LveCamera cam);

  glm::vec3 changeColor();

  LveGameObject viewerObject = LveGameObject::createGameObject();

  LveWindow lveWindow{WIDTH, HEIGHT, "Vulkan Tutorial"};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};

  std::vector<LveGameObject> gameObjects;
  std::vector<LveGameObject> objectList;

};
}  // namespace lve
