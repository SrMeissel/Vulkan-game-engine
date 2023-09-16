#include "app.hpp"
#include "cameraManager.hpp"
#include "keyboard_movement_controller.hpp"
#include "bufferManager.hpp"
#include "systems/renderSystem.hpp"
#include "systems/pointLightSystem.hpp"


#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <String>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {

    app::app() {
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();

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

        auto globalSetLayout = DescriptorSetLayout::Builder(device).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for(int i=0; i < globalDescriptorSets.size(); i++){
            DescriptorWriter writer(*globalSetLayout, *globalPool);

            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, &bufferInfo);
            
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.getTextureImageView();
            imageInfo.sampler = texture.getTextureSampler();
                
            writer.writeImage(1, &imageInfo);
            writer.build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        CameraManager camera{};

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
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


                //render
                renderer.beginSwapChainRenderPass(commandBuffer);

                //order matters, the transparent pointLights need to be rendered first (Brendan tutorial 27)
                renderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    void app::initilizeObject(GameObject& object, glm::vec3 position, glm::vec3 scale, std::string modelFile, char * textureFile) {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, modelFile);
        object.model = model;
        object.transform.translation = position;
        object.transform.scale = scale;
        gameObjects.emplace(object.getId(), std::move(object));
    }

    void app::loadGameObjects() {

        glm::vec3 translation;
        glm::vec3 scale; 

        auto object = GameObject::createGameObject();
        translation = {0.0f, 0.5f, 0.0f};
        scale = {1, 1, 1};
        initilizeObject(object, translation, scale, "models/flat_vase.obj");

        //Second object
        auto secondObject = GameObject::createGameObject();
        translation = {-1.0f, -0.5f, 2.5f};
        scale = {0.5, 0.5, 0.5};
        initilizeObject(secondObject, translation, scale, "models/colored_cube.obj");

        //floor
        auto floor = GameObject::createGameObject();
        translation = {0.0f, 0.5f, 0.0f};
        scale = {5.0, 1.0, 5.0};
        initilizeObject(floor, translation, scale, "models/quad.obj");

        //tree
        auto tree = GameObject::createGameObject();
        translation = {0.0f, 1.5f, 0.0f};
        scale = {1, 1, 1};
        initilizeObject(tree, translation, scale, "models/Lowpoly_tree_sample.obj");

           
        // best light color {1.0f, 0.96f, 0.71f};
        int lightNum = 3;
        float radius = 3.0f;
        for(int i=0; i <lightNum; i++){
            auto pointLight = GameObject::makePointLight();
            pointLight.color = {1.0f, 0.96f, 0.71f};
            auto  rotateLight = glm::rotate(glm::mat4(1.0f), (i*glm::two_pi<float>()) / lightNum, {0.0f, -1.0f, 0.0f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-radius, -1, -radius, 1.0f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}