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
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

        loadGameObjects();
        loadTextures();
    }
    app::~app() {
        //this should probably be moved to texture deconstructor (Textures would have to be completely rearranged, do when looking into bindless textures)
        for(int i=0; i < 3; i ++) {
            textureManager.destroyTexture(loadedTextures[i]);
        }
    }

    void app::run() {
        //initiliaze GPU memory objects ==================================================

        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        int textureAmount = sizeof(loadedTextures)/sizeof(Texture);

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, textureAmount)
        .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < globalDescriptorSets.size(); i++){
            DescriptorWriter writer(*globalSetLayout, *globalPool);

            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, &bufferInfo);

            VkDescriptorImageInfo samplerInfo;
            samplerInfo.sampler = textureManager.getTextureSampler();
            writer.writeImage(1, &samplerInfo, 1);

            VkDescriptorImageInfo imageInfo[sizeof(loadedTextures)/sizeof(Texture)];
            for(int j=0; j < textureAmount; j++){
                imageInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo[j].imageView = loadedTextures[j].imageView;
                imageInfo[j].sampler = textureManager.getTextureSampler();
            }
            writer.writeImages(2, imageInfo, sizeof(loadedTextures)/sizeof(Texture));

            writer.build(globalDescriptorSets[i]);
        }

        //Initialize shader objects ======================================

        RenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        //Initialize Camera object ===================================

        CameraManager camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        float orbitSpeed = 1;
        auto viewerObject = GameObject::createGameObject();
        keyboardMovementController cameraController{};

        //Main system loop ============================================

        auto currentTime = std::chrono::high_resolution_clock::now();
        bool screenshotSaved = false; // <=====
        while(!window.shouldClose()){
            glfwPollEvents();

            //get passed time
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
            currentTime = newTime;

            std::cout << 1.0f/frameTime << "\n";
            //perhaps set upper limit to frameTime to limit edge cases

            //proccess user input =======================================================

            //take screenshot
            int stateKeyP = glfwGetKey(window.getGLFWwindow(), GLFW_KEY_P);
            if(stateKeyP == GLFW_PRESS && screenshotSaved == false) {
                std::vector<VkImage> images = renderer.getSwapchainImages();
                VkImage srcImage = images[renderer.getCurrentImageIndex()]; 
                screenshotTool.takeScreenshot(srcImage, "testScreenshot.jpg", device, window.getExtent());
                screenshotSaved = true;
            }

            //shoot ray forward from camera!
            if(glfwGetKey(window.getGLFWwindow(), GLFW_KEY_SPACE) == GLFW_PRESS) {
                Ray ray;
                ray.posiiton = viewerObject.transform.translation;
                float yaw = viewerObject.transform.rotation.y;
                float pitch = viewerObject.transform.rotation.x;
                ray.direction = {sin(yaw), -tan(pitch), cos(yaw)};
                ray.direction = glm::normalize(ray.direction);

                physicsSimulation.sphereRayCollision(ray, *physicsSimulation.objects[0].collisionMesh.get());
            }

            //update camera from user input
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);            
            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 50.0f);


            //update objects ==========================================================

            physicsSimulation.update(frameTime);

            //new frame ready, runs every frame ===============================================
            if(auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();

                frameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects
                };

                //update graphics memory objects =====================================
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //render =====================================================
                renderer.beginSwapChainRenderPass(commandBuffer);

                //order matters, the transparent pointLights need to be rendered first (Brendan tutorial 27)
                renderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                //finished and submit to presentation
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    void app::initilizeObject(GameObject& object, glm::vec3 position, glm::vec3 scale, std::string modelFile, int textureIndex) {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, modelFile);
        object.model = move(model);
        object.transform.translation = position;
        object.transform.scale = scale;
        object.textureIndex = textureIndex;
        gameObjects.emplace(object.getId(), std::move(object));
    }

    void app::loadGameObjects() {

        glm::vec3 translation;
        glm::vec3 scale; 

        auto object = GameObject::createGameObject(); 
        translation = {0.0f, 0.5f, 0.0f};
        scale = {1, 1, 1};
        initilizeObject(object, translation, scale, "models/flat_vase.obj", 1);

        //Second object
        auto secondObject = GameObject::createGameObject();
        translation = {-1.0f, -0.5f, 2.5f};
        scale = {0.5, 0.5, 0.5};
        initilizeObject(secondObject, translation, scale, "models/sphere.obj");
        //creates a physics sim object
        PhysicsObject physics(gameObjects[1].transform, gameObjects[1].getId());
        physicsSimulation.objects.emplace_back(std::move(physics));

        //floor
        auto floor = GameObject::createGameObject();
        translation = {0.0f, 0.5f, 0.0f};
        scale = {50.0, 1.0, 50.0};
        initilizeObject(floor, translation, scale, "models/quad.obj", 2);

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

    void app::loadTextures() {
        Texture texture;
        texture.image = textureManager.createTextureImage("../../textures/default_texture.jpg");
        texture.imageView = textureManager.createTextureImageView(texture.image);
        loadedTextures[0] = texture;

        texture.image = textureManager.createTextureImage("../../Experimental/Roma Imperiale Granite_whgneh2v/Albedo_8K__whgneh2v.jpg");
        texture.imageView = textureManager.createTextureImageView(texture.image);
        loadedTextures[1] = texture;

        texture.image = textureManager.createTextureImage("../../Experimental/Mossy_Ground_xiboab2r/Albedo_2K__xiboab2r.jpg");
        texture.imageView = textureManager.createTextureImageView(texture.image);
        loadedTextures[2] = texture;
    }
}