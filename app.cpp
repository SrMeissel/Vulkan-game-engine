#include "app.hpp"
#include "renderSystem.hpp"
#include "cameraManager.hpp"
#include "keyboard_movement_controller.hpp"
#include "bufferManager.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {

    struct GlobalUbo {
        glm::mat4 projectionView{1.0f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, 3.0f, -1.0f});
    };

    app::app() {
        loadGameObjects();
    }
    app::~app() {
        
    }

    void app::run() {

        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        RenderSystem renderSystem{device, renderer.getRenderPass()};
        CameraManager camera{};
        //camera.setViewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = GameObject::createGameObject();
        keyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!window.shouldClose()){
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
            currentTime = newTime;

            //update camera
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            //rotate second game object
            gameObjects[1].transform.rotation = glm::mod((gameObjects[1].transform.rotation + (0.1f*frameTime)), glm::two_pi<float>());
            
            float aspect = renderer.getAspectRatio();
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 5);
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

            if(auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                frameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer, camera
                };

                //update buffer
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                //render
                renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo, gameObjects);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    void app::loadGameObjects() {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "C:/Users/Ethan Mizer/Documents/Projects/Vulkan-Engine/Vulkan-game-engine/models/flat_vase.obj");
        auto object = GameObject::createGameObject();
        object.model = model;
        object.transform.translation = {1.0f, 0.5f, 2.5f};
        object.transform.scale = {1, 1, 1};
        gameObjects.push_back(std::move(object));

        //Second object
        model = Model::createModelFromFile(device, "C:/Users/Ethan Mizer/Documents/Projects/Vulkan-Engine/Vulkan-game-engine/models/colored_cube.obj");
        auto secondObject = GameObject::createGameObject();
        secondObject.model = model;
        secondObject.transform.translation = {-1.0f, 0.5f, 2.5f};
        secondObject.transform.scale = {0.5, 0.5, 0.5};
        gameObjects.push_back(std::move(secondObject));

    }
}