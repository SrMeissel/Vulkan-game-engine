#include "app.hpp"
#include "cameraManager.hpp"
#include "keyboard_movement_controller.hpp"
#include "bufferManager.hpp"
#include "systems/renderSystem.hpp"
#include "systems/pointLightSystem.hpp"
#include "systems/Volumetric/AtmoSystem.hpp"

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
        loadGameObjects();
    }
    app::~app() {

    }

    void app::run() {
        //initiliaze GPU memory objects ==================================================

        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();

        //init UBO
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }
        // add UBO to descriptor
        auto globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < globalDescriptorSets.size(); i++){
            DescriptorWriter writer(*globalSetLayout, *globalPool);

            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            writer.writeBuffer(0, &bufferInfo);
            writer.build(globalDescriptorSets[i]);
        }

        //Initialize render systems ======================================

        RenderPass scenePass{device, window, configureRenderPass(), true};
        renderer.appendRenderPass(scenePass);

        RenderSystem renderSystem{device, renderer.getRenderPass(0).getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{device, renderer.getRenderPass(0).getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        //AtmoSystem atmoSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        //Initialize Camera object ===================================

        CameraManager camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        float orbitSpeed = 1;
        auto viewerObject = GameObject::createGameObject();
        keyboardMovementController cameraController{};

        //Main system loop ============================================

        auto currentTime = std::chrono::high_resolution_clock::now();
        bool screenshotSaved = false; // <=====
        //int frames = 0; // <=== useful for debugging (add to while condition)
        while(!window.shouldClose()){
            glfwPollEvents();

            //Updates scene editor frame, should not stay here.
            //sceneEditor.run();

            //get passed time
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
            currentTime = newTime;

            //std::cout << 1.0f/frameTime << "\n";
            //perhaps set upper limit to frameTime so the program doesnt combust at low fps 

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

                renderer.beginNextRenderPass(commandBuffer);
                //order matters, the transparent pointLights need to be rendered first (Brendan tutorial 27)
                renderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                //atmoSystem.renderAtmosphere(frameInfo, renderer.getSwapchainDepthImageViews()[renderer.getCurrentImageIndex()]);
                //this works, just not focusing on it rn 

                vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

                renderer.endCurrentRenderPass(commandBuffer);
                renderer.beginSwapChainRenderPass(commandBuffer);

                sceneEditor.run(commandBuffer);

                //finished and submit to presentation
                renderer.endSwapChainRenderPass(commandBuffer);
                
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    void app::initilizeObject(GameObject& object, glm::vec3 position, glm::vec3 scale, std::string modelFile, char* textureFile) {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, modelFile);
        object.model = move(model);
        object.transform.translation = position;
        object.transform.scale = scale;

        std::shared_ptr<Texture> texture = textureManager.createTextureFromFile(textureFile);
        object.texture = move(texture);

        gameObjects.emplace(object.getId(), std::move(object));
    }

    void app::loadGameObjects() {

        glm::vec3 translation;
        glm::vec3 scale; 

        auto object = GameObject::createGameObject(); 
        translation = {0.0f, 0.5f, 0.0f};
        scale = {1, 1, 1};
        initilizeObject(object, translation, scale, "models/flat_vase.obj", "../../textures/default_texture.jpg");  

        //Second object
        auto secondObject = GameObject::createGameObject();
        translation = {-1.0f, -0.5f, 2.5f};
        scale = {0.5, 0.5, 0.5};
        initilizeObject(secondObject, translation, scale, "models/sphere.obj", "../../Experimental/Roma Imperiale Granite_whgneh2v/Albedo_8K__whgneh2v.jpg");
        //creates a physics sim object
        PhysicsObject physics(gameObjects[1].transform, gameObjects[1].getId());
        physicsSimulation.objects.emplace_back(std::move(physics));

        //floor
        auto floor = GameObject::createGameObject();
        translation = {0.0f, 0.5f, 0.0f};
        scale = {50.0, 1.0, 50.0};
        initilizeObject(floor, translation, scale, "models/quad.obj", "../../Experimental/Mossy_Ground_xiboab2r/Albedo_2K__xiboab2r.jpg");

        //tree
        auto tree = GameObject::createGameObject();
        translation = {0.0f, 1.5f, 0.0f};
        scale = {1, 1, 1};
        initilizeObject(tree, translation, scale, "models/Lowpoly_tree_sample.obj", "../../textures/default_texture.jpg");

           
        // best light color {1.0f, 0.96f, 0.71f};
        int lightNum = 1;
        float radius = 2.5f;
        for(int i=0; i <lightNum; i++){
            auto pointLight = GameObject::makePointLight();
            pointLight.color = {1.0f, 0.96f, 0.71f};
            auto  rotateLight = glm::rotate(glm::mat4(1.0f), (i*glm::two_pi<float>()) / lightNum, {0.0f, -1.0f, 0.0f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-radius, -1, -radius, 1.0f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }

    //this works, vkcreateRenderPass uses pointer. The static keywords are used to prevent the objects from deleteing because their referenced.
    VkRenderPassCreateInfo* app::configureRenderPass() {

        static std::array<VkAttachmentDescription, 2> attachments;
        //colorAttachment (Attachment 0 must be swapchain image, this is constant every time)
            attachments[0].format = chooseSwapSurfaceFormat();
            attachments[0].samples = device.msaaSamples;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        //depthAttachment
            attachments[1].format = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            attachments[1].samples = device.msaaSamples;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        static VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        static VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        //for second subpass
        static VkAttachmentReference inputReference = {};
        inputReference.attachment = 1;
        inputReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static std::array<VkSubpassDescription, 2> subpasses {};
            subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[0].colorAttachmentCount = 1;
            subpasses[0].pColorAttachments = &colorAttachmentRef;
            subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

            subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[1].colorAttachmentCount = 1;
            subpasses[1].pColorAttachments = &colorAttachmentRef;
            subpasses[1].inputAttachmentCount = 1;
            subpasses[1].pInputAttachments = &inputReference;

        static std::array<VkSubpassDependency, 3> dependency = {};
            dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency[0].dstSubpass = 0;
            dependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependency[1].srcSubpass = 0;
            dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependency[2].srcSubpass = 0;
            dependency[2].dstSubpass = 1;
            dependency[2].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency[2].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            dependency[2].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            dependency[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependency[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo* renderPassInfo = new VkRenderPassCreateInfo();
        renderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo->attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo->pAttachments = attachments.data();
        renderPassInfo->subpassCount = subpasses.size();
        renderPassInfo->pSubpasses = subpasses.data();
        renderPassInfo->dependencyCount = dependency.size();
        renderPassInfo->pDependencies = dependency.data();

        return renderPassInfo;
    }

    VkFormat app::chooseSwapSurfaceFormat() {
        std::vector<VkSurfaceFormatKHR> availableFormats = device.getSwapChainSupport().formats;
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat.format;
            }
        }

        return availableFormats[0].format;
        }
}