#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"

#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

# define M_PI           3.14159265358979323846  /* pi */

// std
#include <array>
#include <iostream>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lve {

    FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
  glm::vec3 color = { .1f, .8f, .1f };

  // Camera
  LveCamera camera{};

  KeyboardMovementController keyboardController{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  auto fpsTime = std::chrono::high_resolution_clock::now();
  int counter = 0;

  // Game Loop
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();
    
    // Check for mouse lock toggle
    // More annoying than anything without using mouse for camera; disabled
    //if (keyboardController.toggleMouseLock(lveWindow.getGLFWwindow())) lveWindow.toggleMouse();

    // Import Model
    if (keyboardController.importModelCheck(lveWindow.getGLFWwindow())) importModel();

    // Place Voxel
    if (keyboardController.placeVoxelCheck(lveWindow.getGLFWwindow())) makeCube(camera, color);

    // Remove voxel
    if (keyboardController.removeVoxelCheck(lveWindow.getGLFWwindow())) removeCube(camera);

    // Change voxel color
    if (keyboardController.updateColorCheck(lveWindow.getGLFWwindow())) color = changeColor();

    // Reset Camarea
    if (keyboardController.resetCameraCheck(lveWindow.getGLFWwindow())) {
        viewerObject.transform.translation = { 0,0,0 };
        viewerObject.transform.rotation = { 0,0,0 };
    }

   

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    keyboardController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    cull(camera);

    if (std::chrono::duration<float, std::chrono::seconds::period>(newTime - fpsTime).count() >= 1) {
        std::cout << "Game Loops Per Second: " << floor(counter / std::chrono::duration<float, std::chrono::seconds::period>(newTime - fpsTime).count()) << std::endl;
        std::cout << "Game Objects loaded: " << objectList.size() << std::endl << std::endl;
        fpsTime = newTime;
        counter = 0;
    }
    
    if (auto commandBuffer = lveRenderer.beginFrame()) {
      lveRenderer.beginSwapChainRenderPass(commandBuffer);

      simpleRenderSystem.renderGameObjects(commandBuffer, objectList, camera);

      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
    counter++;
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::cull(LveCamera cam) {
    objectList.clear();
    glm::mat4 camMatrix = cam.getProjection() * cam.getView();
    for (int i = 0; i < gameObjects.size(); i++) {
        auto gameObj = LveGameObject::createGameObject();
        gameObj.model = gameObjects[i].model;
        gameObj.transform = gameObjects[i].transform;
        gameObj.color = gameObjects[i].color;

        // Get position of center of cube relative to the camera
        glm::vec3 frameLocation = camMatrix * gameObj.transform.mat4() * glm::vec4(gameObj.transform.translation, 1);

        // Frustum Calling
        if ((abs(frameLocation.x) <= 7*glm::length(frameLocation)/8 && abs(frameLocation.y) <= 7*glm::length(frameLocation)/8 && frameLocation.z >= -0.5) || glm::length(frameLocation) < 2) objectList.push_back(std::move(gameObj));
    }
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset, glm::vec3 color = { .1f, .8f, .1f }) {
    LveModel::Builder modelBuilder{};
    modelBuilder.vertices = {
        // back face
        {{-.5f, -.5f, .5f}, color + color*0.1f},
        {{.5f, .5f, .5f}, color},
        {{.5f, -.5f, .5f}, color},
        {{-.5f, .5f, .5f}, color},

        // front face
        {{.5f, .5f, -.5f}, color + color*0.1f},
        {{-.5f, -.5f, -.5f}, color},
        {{.5f, -.5f, -.5f}, color},
        {{-.5f, .5f, -.5f}, color},
    };

    for (auto& v : modelBuilder.vertices) {
        v.position += offset;
    }

    modelBuilder.indices = { 0, 1, 2, 0, 1, 3, 4, 5, 6, 4, 5, 7, // front and back
                             1, 2, 4, 4, 6, 2, 0, 3, 7, 5, 7, 0, // left and right
                             0, 2, 6, 0, 6, 5, 4, 7, 3, 4, 1, 3 // top and bottom
                            };

  return std::make_unique<LveModel>(device, modelBuilder);
}

void FirstApp::importModel() {
    std::string filepath;

    std::cout << "Path to file: ";
    std::cin >> filepath;

    glm::vec3 globalPos;

    std::cout << "X offset: ";
    std::cin >> globalPos.x;

    std::cout << "Y offset: ";
    std::cin >> globalPos.y;

    std::cout << "Z offset: ";
    std::cin >> globalPos.z;

    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, filepath);
    auto cube = LveGameObject::createGameObject();
    cube.model = lveModel;
    cube.transform.translation = { globalPos.x, globalPos.y, globalPos.z };
    cube.transform.scale = { .25f, .25f, .25f };
    gameObjects.push_back(std::move(cube));
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<LveModel> lveModel = LveModel::createVoxelFromFile(lveDevice, {0.1f, 0.8f, 0.1f});
  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {.0f, .0f, 2.5f};
  cube.transform.scale = {.25f, .25f, .25f};
  gameObjects.push_back(std::move(cube));
}

void FirstApp::removeCube(LveCamera cam) {
    // Find cube closest to camera
    int closestCube = 0;
    if (gameObjects.size() > 1) {
        glm::vec3 distanceVec = cam.getPosition() - gameObjects.at(closestCube).transform.translation;

        for (int i = 1; i < gameObjects.size(); i++)
            if (glm::length(distanceVec) > glm::length(cam.getPosition() - gameObjects.at(i).transform.translation)) {
                closestCube = i;
                distanceVec = cam.getPosition() - gameObjects.at(closestCube).transform.translation;
            }
        vkDeviceWaitIdle(lveDevice.device()); // Wait until object is free from command buffer before removing
        gameObjects.erase(gameObjects.begin() + closestCube);
    }
    
}

glm::vec3 FirstApp::changeColor() {
    glm::vec3 color{ 0.f };
    float c_val = 0;

    std::cout << "R Value: ";
    std::cin >> c_val;
    color.x = c_val;

    std::cout << "G Value: ";
    std::cin >> c_val;
    color.y = c_val;

    std::cout << "B Value: ";
    std::cin >> c_val;
    color.z = c_val;

    std::cout << "Color set!\n\n";

    return color;
}

void FirstApp::makeCube(LveCamera cam, glm::vec3 color) {
    // Find cube closest to camera
    int closestCube = 0;
    glm::vec3 camDir = { cam.getView()[0][2], cam.getView()[1][2], cam.getView()[2][2]};
    glm::vec3 distanceVec = cam.getPosition() - gameObjects.at(closestCube).transform.translation;

    bool lookAtCube = false;

    for (int i = 0; i < gameObjects.size(); i++) {
        if (glm::length(distanceVec) > glm::length(gameObjects.at(i).transform.translation - cam.getPosition()) && !lookAtCube) {
            closestCube = i;
            distanceVec = -gameObjects.at(i).transform.translation + cam.getPosition();
        }
    }

    distanceVec = glm::normalize(distanceVec);
    glm::vec3 unitVec{ 0.f };

    // Find a unit vector based on player position and cube position
    if (glm::length(distanceVec) == 0.f) unitVec.x = 1.f;
    else if (abs(distanceVec.x) > abs(distanceVec.y) && abs(distanceVec.x) > abs(distanceVec.z)) unitVec.x = 1.f * distanceVec.x / abs(distanceVec.x);
    else if (abs(distanceVec.y) > abs(distanceVec.z)) unitVec.y = 1.f * distanceVec.y / abs(distanceVec.y);
    else unitVec.z = 1.f * distanceVec.z / abs(distanceVec.z);

    std::shared_ptr<LveModel> cubeModel = LveModel::createVoxelFromFile(lveDevice, color);

    auto cube = LveGameObject::createGameObject();
    cube.model = cubeModel;
    cube.transform.translation = gameObjects.at(closestCube).transform.translation + (unitVec * 0.5f);
    cube.transform.rotation = gameObjects.at(closestCube).transform.rotation;
    cube.transform.scale = { .25f, .25f, .25f };
    gameObjects.push_back(std::move(cube));
}

}  // namespace lve
