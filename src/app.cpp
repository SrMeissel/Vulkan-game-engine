#include "app.hpp"
#include "systems/renderSystem.hpp"
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
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        //glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});

        glm::vec4 ambientColor{1.0f, 1.0f, 1.0f, 0.02f};

        glm::vec3 lightPosition{-1.0f}; 
        alignas(16) glm::vec4 lightColor{1.0f,0.96f,0.71f,2.0f};
    };

    app::app() {
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();
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

        auto globalSetLayout = DescriptorSetLayout::Builder(device).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        CameraManager camera{};
        //camera.setViewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        float orbitSpeed = 1;

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
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 50.0f);

            if(auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                frameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects
                };




                //update buffer
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                orbitSpeed = glm::mod(orbitSpeed+(1.0f*frameTime), glm::two_pi<float>());
                ubo.lightPosition.x = cos(orbitSpeed)*2;
                ubo.lightPosition.z = sin(orbitSpeed)*2;
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                //render
                renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    void app::loadGameObjects() {
        //looks like I can make these into another function! hahahahahahaha this is going quite splendid indeed!
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/flat_vase.obj");
        auto object = GameObject::createGameObject();
        object.model = model;
        object.transform.translation = {0.0f, 0.5f, 0.0f};
        object.transform.scale = {1, 1, 1};
        gameObjects.emplace(object.getId(), std::move(object));

        //Second object
        model = Model::createModelFromFile(device, "models/colored_cube.obj");
        auto secondObject = GameObject::createGameObject();
        secondObject.model = model;
        secondObject.transform.translation = {-1.0f, -0.5f, 2.5f};
        secondObject.transform.scale = {0.5, 0.5, 0.5};
        gameObjects.emplace(secondObject.getId(), std::move(secondObject));

        //floor
        model = Model::createModelFromFile(device, "models/quad.obj");
        auto floor = GameObject::createGameObject();
        floor.model = model;
        floor.transform.translation = {0.0f, 0.5f, 0.0f};
        floor.transform.scale = {5.0, 1.0, 5.0};
        gameObjects.emplace(floor.getId(), std::move(floor));

    }
}