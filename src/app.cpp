#include "app.hpp"
#include "cameraManager.hpp"
#include "bufferManager.hpp"
#include "ECS/Importer.hpp"
#include "frameInfo.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#ifndef ENGINE_PATH
#define ENGINE_PATH "C:/Users/mizer/dev/vulkan-game-engine/"
#endif

namespace engine {

    app::app() {
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

        createSamplers();

        //Initialize render systems ======================================

        RenderPass scenePass{device, window, configureRenderPass(), false, {800, 600}};
        renderer.appendRenderPass(& scenePass);

        //Initialize asset system ======================================
        assetSystem.Init();

        assetSystem.RegisterComponent<ECS::Transform>();
        assetSystem.RegisterComponent<ECS::Renderable>();
        assetSystem.RegisterComponent<ECS::Material>();
        assetSystem.RegisterComponent<ECS::Script>();
        assetSystem.RegisterComponent<ECS::PointLight>();

        std::shared_ptr<MeshSystem> meshSystem = assetSystem.RegisterSystem<MeshSystem>(device, renderer.getRenderPass(0)->getRenderPass(), globalSetLayout->getDescriptorSetLayout());
        std::shared_ptr<ScriptingSystem> scriptingSystem = assetSystem.RegisterSystem<ScriptingSystem>(window);
        std::shared_ptr<PointLightSystem> pointLightSystem = assetSystem.RegisterSystem<PointLightSystem>(device, renderer.getRenderPass(0), globalSetLayout->getDescriptorSetLayout());
        SkyboxSystem skyboxSystem{device, renderer.getRenderPass(0)->getRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        ECS::Signature meshSignature;
        meshSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        meshSignature.set(assetSystem.GetComponentType<ECS::Renderable>());
        assetSystem.SetSystemSignature<MeshSystem>(meshSignature);

        ECS::Signature meshAntiSignature;
        meshAntiSignature.set(assetSystem.GetComponentType<ECS::Material>());
        assetSystem.SetSystemAntiSignature<MeshSystem>(meshAntiSignature);

        std::shared_ptr<MaterialSystem> materialSystem = assetSystem.RegisterSystem<MaterialSystem>(device, renderer.getRenderPass(0)->getRenderPass(), globalSetLayout->getDescriptorSetLayout());

        ECS::Signature materialSignature;
        materialSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        materialSignature.set(assetSystem.GetComponentType<ECS::Renderable>());
        materialSignature.set(assetSystem.GetComponentType<ECS::Material>());
        assetSystem.SetSystemSignature<MaterialSystem>(materialSignature);

        ECS::Signature scriptSignature;
        scriptSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        scriptSignature.set(assetSystem.GetComponentType<ECS::Script>());
        assetSystem.SetSystemSignature<ScriptingSystem>(scriptSignature);

        ECS::Signature pointLightSignature;
        pointLightSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        pointLightSignature.set(assetSystem.GetComponentType<ECS::PointLight>());
        assetSystem.SetSystemSignature<PointLightSystem>(pointLightSignature);

        ECS::SaveDataManager saveDataManager{device, *scriptingSystem, *materialSystem}; 
        saveDataManager.loadData("../../saveFiles/statuette.xml", assetSystem);

        // ECS::Entity pointLight2 = assetSystem.CreateEntity();
        // assetSystem.AddComponent(pointLight2, ECS::Transform{glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.5f), glm::vec3(0.0f)});
        // assetSystem.AddComponent(pointLight2, ECS::Renderable("../../models/sphere.obj", device));
        // assetSystem.AddComponent(pointLight2, ECS::PointLight{glm::vec3(1.0f, 0.0f, 0.0f)});


        ECS::Entity pointLight = assetSystem.CreateEntity();
        assetSystem.AddComponent(pointLight, ECS::Transform{glm::vec3(0.0f, -0.5f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)});
        assetSystem.AddComponent(pointLight, ECS::Script{"TransformExpirement", scriptingSystem->assembly, scriptingSystem->appDomain});
        assetSystem.AddComponent(pointLight, ECS::PointLight{glm::vec3(1.0f, 0.96f, 0.71f), 1.0f});


        //saveDataManager.saveData("../../saveFiles/statuette.xml", assetSystem.getAllEntities());


        //=======================================================================

        sceneEditor.configureViewport(renderer.getRenderPass(0)->getAttachmentImageView(4), renderer.getRenderPass(0)->getAttachmentImageView(1), sampler, renderer.getRenderPass(0)->extent);
 
        //Initialize Camera object ===================================

        CameraManager camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        ECS::Entity viewerObject = assetSystem.CreateEntity();
        assetSystem.AddComponent(viewerObject, ECS::Transform{glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)});
        assetSystem.AddComponent(viewerObject, ECS::Script{"CameraControl", scriptingSystem->assembly, scriptingSystem->appDomain });

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

            
            scriptingSystem->update(frameTime, assetSystem);

            //take screenshot
            int stateKeyP = glfwGetKey(window.getGLFWwindow(), GLFW_KEY_P);
            if(stateKeyP == GLFW_PRESS && screenshotSaved == false) {
                std::vector<VkImage> images = renderer.getSwapchainImages();
                VkImage srcImage = images[renderer.getCurrentImageIndex()]; 
                screenshotTool.takeScreenshot(srcImage, "testScreenshot.jpg", device, window.getExtent());
                screenshotSaved = true;
            }

            //update camera from user input
            ECS::Transform& viewerTransform = assetSystem.GetComponent<ECS::Transform>(viewerObject);
            //cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerTransform);
            camera.setViewYXZ(viewerTransform.translation, viewerTransform.rotation);            
            float aspect = renderer.getRenderPass(0)->getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 500.0f);

            //new frame ready, runs every frame ===============================================
            if(auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();

                frameInfo frameInfo{
                    frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]
                };

                //update graphics memory objects =====================================
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //render =====================================================

                renderer.beginNextRenderPass(commandBuffer);

                meshSystem->Render(commandBuffer, globalDescriptorSets[frameIndex], assetSystem);
                materialSystem->Render(commandBuffer, globalDescriptorSets[frameIndex], assetSystem);

                vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

                pointLightSystem->Render(commandBuffer, globalDescriptorSets[frameIndex], assetSystem);
                // skyboxSystem.Render(commandBuffer, globalDescriptorSets[frameIndex], assetSystem, {0.0f, 0.0f, 1.0f, 1.0f});
                // skyboxSystem.Render(commandBuffer, globalDescriptorSets[frameIndex], assetSystem, {1.0f, 0.0f, 0.0f, 1.0f});

                renderer.endCurrentRenderPass(commandBuffer);
                renderer.beginSwapChainRenderPass(commandBuffer);


                sceneEditor.run(commandBuffer);

                //finished and submit to presentation
                renderer.endSwapChainRenderPass(commandBuffer);
                
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());

        //DESTROY EVERYTHING ==================================================================================================
        //I don't know how.
        //nvm im a genius

        materialSystem->cleanup(assetSystem);

        vkDestroySampler(device.device(), sampler, nullptr);
        vkDestroySampler(device.device(), cubeSampler, nullptr);

    }

    //this works, vkcreateRenderPass uses pointer. The static keywords are used to prevent the objects from deleteing because their referenced.
    VkRenderPassCreateInfo* app::configureRenderPass() {

        static std::array<VkAttachmentDescription, 5> attachments;

        static std::array<VkAttachmentReference, 3> colorAttachmentRef = {};
        static std::array<VkAttachmentReference, 3> inputReference = {};


        //colorAttachment (Attachment 0 must be swapchain image, this is constant every time)
        attachments[0].format = chooseSwapSurfaceFormat();
        attachments[0].samples = device.msaaSamples;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // <==================

        colorAttachmentRef[0].attachment = 0;
        colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[0].attachment = 0;
        inputReference[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //normal color attachment
        attachments[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // <================== device specific
        attachments[1].samples = device.msaaSamples;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachmentRef[1].attachment = 1;
        colorAttachmentRef[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[1].attachment = 1;
        inputReference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //position color attachment
        attachments[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;  // <================== device specific
        attachments[2].samples = device.msaaSamples;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachmentRef[2].attachment = 2;
        colorAttachmentRef[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        inputReference[2].attachment = 2;
        inputReference[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //depthAttachment
        attachments[3].format = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachments[3].samples = device.msaaSamples;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 3;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //lighting attachment
        attachments[4].format = chooseSwapSurfaceFormat();
        attachments[4].samples = device.msaaSamples;
        attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[4].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static VkAttachmentReference lightingAttachmentRef = {};
        lightingAttachmentRef.attachment = 4;
        lightingAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        static std::array<VkSubpassDescription, 2> subpasses {};
            subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[0].colorAttachmentCount = colorAttachmentRef.size();
            subpasses[0].pColorAttachments = colorAttachmentRef.data();
            subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

            subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[1].colorAttachmentCount = 1;
            subpasses[1].pColorAttachments = &lightingAttachmentRef;
            subpasses[1].inputAttachmentCount = inputReference.size();
            subpasses[1].pInputAttachments = inputReference.data();
            subpasses[1].pDepthStencilAttachment = &depthAttachmentRef;

        static std::array<VkSubpassDependency, 1> dependency = {};
            // dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            // dependency[0].dstSubpass = 0;
            // dependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            // dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            // dependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            // dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            // dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            // dependency[1].srcSubpass = 0;
            // dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            // dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            // dependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            // dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            // dependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            // dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependency[0].srcSubpass = 0;
            dependency[0].dstSubpass = 1;
            dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage of writing to the color attachment
            dependency[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Stage of reading from the attachment in the shader
            dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Access type for writing to the color attachment
            dependency[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; 
            dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

    void app::createSamplers() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);
        VkSamplerCreateInfo samplerInfo{};

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        
        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        
        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &cubeSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
}