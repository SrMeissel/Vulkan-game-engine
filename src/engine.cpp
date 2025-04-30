#include "engine.hpp"
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

namespace engine {

    engine::engine(renderer::Renderer& renderer, ECS::AssetSystem& assetSystem) : renderer{renderer}, assetSystem{assetSystem} {
        //Initialize render systems ======================================
        //TODO: I should start adding TODO's around my project.

        meshSystem = assetSystem.RegisterSystem<MeshSystem>(renderer.device, renderer.primaryRenderPass->renderPass, renderer.globalSetLayout->getDescriptorSetLayout());
        materialSystem = assetSystem.RegisterSystem<MaterialSystem>(renderer.device, renderer.primaryRenderPass->renderPass, renderer.globalSetLayout->getDescriptorSetLayout());
        scriptingSystem = assetSystem.RegisterSystem<ScriptingSystem>(renderer.window);
        pointLightSystem = assetSystem.RegisterSystem<PointLightSystem>(renderer.device, renderer.primaryRenderPass.get(), renderer.globalSetLayout->getDescriptorSetLayout());
        spotLightSystem = assetSystem.RegisterSystem<SpotLightSystem>(renderer.device, renderer.primaryRenderPass.get(), renderer.globalSetLayout->getDescriptorSetLayout());
        skyboxSystem = assetSystem.RegisterSystem<SkyboxSystem>(renderer.device, renderer.primaryRenderPass->renderPass, renderer.globalSetLayout->getDescriptorSetLayout());

        //I need a list of all renderable objects for shadows. This makes me want to detach the entity list from systems, It would be a lot more simple.
        renderables = assetSystem.RegisterSystem<Renderables>();

        ECS::Signature renderablesSignature;
        renderablesSignature.set(assetSystem.GetComponentType<ECS::Renderable>());
        renderablesSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        assetSystem.SetSystemSignature<Renderables>(renderablesSignature);

        ECS::Signature meshSignature;
        meshSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        meshSignature.set(assetSystem.GetComponentType<ECS::Renderable>());
        assetSystem.SetSystemSignature<MeshSystem>(meshSignature);

        ECS::Signature meshAntiSignature;
        meshAntiSignature.set(assetSystem.GetComponentType<ECS::Material>());
        assetSystem.SetSystemAntiSignature<MeshSystem>(meshAntiSignature);

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

        ECS::Signature spotlightSignature;
        spotlightSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        spotlightSignature.set(assetSystem.GetComponentType<ECS::SpotLight>());
        assetSystem.SetSystemSignature<SpotLightSystem>(spotlightSignature);

        ECS::Signature skyboxSigniture;
        pointLightSignature.set(assetSystem.GetComponentType<ECS::Transform>());
        skyboxSigniture.set(assetSystem.GetComponentType<ECS::SkyBox>());
        assetSystem.SetSystemSignature<SkyboxSystem>(skyboxSigniture);

        ECS::SaveDataManager saveDataManager{renderer.device, *scriptingSystem, *materialSystem, *skyboxSystem}; 
        saveDataManager.loadData("/saveFiles/default.xml", assetSystem);
        // saveDataManager.saveData(fileName, assetSystem.getAllEntities())

        ECS::Entity backplane = assetSystem.CreateEntity();
        assetSystem.AddComponent<ECS::Transform>(backplane, ECS::Transform{glm::vec3(0.0f, 0.0f, 25.0f), glm::vec3(40.0f, 1.0f, 40.0f), glm::vec3{glm::radians(90.0f), 0.0f, 0.0f}});
        assetSystem.AddComponent<ECS::Renderable>(backplane, Importer::loadMesh("/models/quad.obj", renderer.device));

        //<Translation x="0" y="-1.5" z="-3"/>
        //glm::vec3(-4.0f, -3.5f, -12.0f)

        glm::vec3 direction = glm::normalize(glm::vec3(0.0, -1.5, -3) - glm::vec3(-4.0, -3.5, -12.0));
        float yaw = atan2(direction.z, direction.x);
        float pitch = atan2(direction.y, sqrt((direction.x * direction.x) + (direction.z * direction.z)));

        ECS::Entity spotlight = assetSystem.CreateEntity();
        assetSystem.AddComponent<ECS::Transform>(spotlight, ECS::Transform{glm::vec3(-4.0f, -3.5f, -12.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(pitch, yaw, 0.0)});
        assetSystem.AddComponent<ECS::SpotLight>(spotlight, ECS::SpotLight{renderer.device, renderer.window, glm::vec3{1.0f, 0.0f, 0.0f}, 500.0f, glm::vec2{800, 600}, spotLightSystem->getRenderPass(), spotLightSystem->getSampler(), spotLightSystem->getSetLayout()});
        assetSystem.AddComponent<ECS::Camera>(spotlight, ECS::Camera{0.1, 500.0});
        assetSystem.AddComponent<ECS::Script>(spotlight, ECS::Script{"TransformExpirement", scriptingSystem->assembly, scriptingSystem->appDomain});

        //=======================================================================

        //Initialize Camera object ===================================

        // viewerObject = assetSystem.CreateEntity();
        // ECS::Transform& viewerTransform = assetSystem.AddComponent(viewerObject, ECS::Transform{glm::vec3(0.0f, -3.5f, -12.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)});
        // ECS::Camera& viewerCamera = assetSystem.AddComponent(viewerObject, ECS::Camera{0.1, 5000});
        // assetSystem.AddComponent(viewerObject, ECS::Script{"CameraControl", scriptingSystem->assembly, scriptingSystem->appDomain});

        // //ECS::Camera& viewerCamera = assetSystem.GetComponent<ECS::Camera>(viewerObject);
        // //ECS::Transform& viewerTransform = assetSystem.GetComponent<ECS::Transform>(viewerObject);
        
        // //viewerCamera.viewMatrix = setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));
        
        // viewerCamera.viewMatrix = setViewYXZ(viewerTransform.translation, viewerTransform.rotation);            
        // float aspect = renderer.getRenderPass(0)->getAspectRatio();
        // viewerCamera.projectionMatrix = setPerspectiveProjection(glm::radians(50.0f), aspect, viewerCamera.nearPlane, viewerCamera.farPlane);
        // viewerCamera.inverseViewMatrix = glm::inverse(viewerCamera.viewMatrix);

    }

    engine::~engine() {
        //DESTROY EVERYTHING ==================================================================================================
        //I don't know how.
        //nvm im a genius

        materialSystem->cleanup(assetSystem);
        skyboxSystem->cleanup(assetSystem);
        spotLightSystem->cleanup(assetSystem);
    }

    void engine::updateGameState(float deltaTime) {
        //proccess user input =======================================================
        
        scriptingSystem->update(deltaTime, assetSystem);

        //take screenshot
        int stateKeyP = glfwGetKey(renderer.window.getGLFWwindow(), GLFW_KEY_P);
        if(stateKeyP == GLFW_PRESS) {
            // std::vector<VkImage> images = renderer.getSwapchainImages();
            // VkImage srcImage = images[renderer.getCurrentImageIndex()]; 
            // screenshotTool.takeScreenshot(srcImage, "testScreenshot.jpg", renderer.device, renderer.window.getExtent());
        }

        //update camera from user input
        ECS::Transform& viewerTransform = assetSystem.GetComponent<ECS::Transform>(viewerObject);
        ECS::Camera& viewerCamera = assetSystem.GetComponent<ECS::Camera>(viewerObject);
        //cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerTransform);
        viewerCamera.viewMatrix = setViewYXZ(viewerTransform.translation, viewerTransform.rotation);            
        float aspect = renderer.primaryRenderPass->getAspectRatio();
        viewerCamera.projectionMatrix = setPerspectiveProjection(glm::radians(50.0f), aspect, viewerCamera.nearPlane, viewerCamera.farPlane);
        viewerCamera.inverseViewMatrix = glm::inverse(viewerCamera.viewMatrix);
    }

    void engine::renderGameState(const ECS::Camera& target) {
    if (target.projectionMatrix == glm::mat4{1.0f}) throw std::runtime_error("you didnt init the camera matricies...");       

	auto commandBuffer = renderer.beginFrame();
	int frameIndex = renderer.getFrameIndex();

        //update graphics memory objects =====================================
        renderer::GlobalUbo ubo{};
        ubo.projection = target.projectionMatrix;
        ubo.view = target.viewMatrix;
        ubo.inverseView = target.inverseViewMatrix;

        renderer.uboBuffers[frameIndex]->writeToBuffer(&ubo);
        renderer.uboBuffers[frameIndex]->flush();

        //render =====================================================

        spotLightSystem->RenderShadows(commandBuffer, renderer.globalDescriptorSets[frameIndex], assetSystem, *renderables);

        renderer.beginRenderPass(commandBuffer, renderer::DefinedRenderPasses::Primary);

        meshSystem->Render(commandBuffer, renderer.globalDescriptorSets[frameIndex], assetSystem);
        materialSystem->Render(commandBuffer, renderer.globalDescriptorSets[frameIndex], assetSystem);

        vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

        pointLightSystem->Render(commandBuffer, renderer.globalDescriptorSets[frameIndex], assetSystem);
        spotLightSystem->RenderLight(commandBuffer, target, assetSystem);
        skyboxSystem->Render(commandBuffer, renderer.globalDescriptorSets[frameIndex], assetSystem);


        renderer.endRenderPass(commandBuffer);

	renderer.endFrame();
    }

    //this works, vkcreateRenderPass uses pointer. The static keywords are used to prevent the objects from deleteing because their referenced.
    VkRenderPassCreateInfo* engine::configureRenderPass() {

        static std::array<VkAttachmentDescription, 5> attachments;

        static std::array<VkAttachmentReference, 3> colorAttachmentRef = {};
        static std::array<VkAttachmentReference, 3> inputReference = {};


        //colorAttachment
        attachments[0].format = chooseSwapSurfaceFormat();
        attachments[0].samples = renderer.device.msaaSamples;
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
        attachments[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // <================== renderer.device specific
        attachments[1].samples = renderer.device.msaaSamples;
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
        attachments[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;  // <================== renderer.device specific
        attachments[2].samples = renderer.device.msaaSamples;
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
        attachments[3].format = renderer.device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachments[3].samples = renderer.device.msaaSamples;
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
        attachments[4].samples = renderer.device.msaaSamples;
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
            subpasses[0].colorAttachmentCount = (int)colorAttachmentRef.size();
            subpasses[0].pColorAttachments = colorAttachmentRef.data();
            subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

            subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpasses[1].colorAttachmentCount = 1;
            subpasses[1].pColorAttachments = &lightingAttachmentRef;
            subpasses[1].inputAttachmentCount = (int)inputReference.size();
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
        renderPassInfo->subpassCount = (int)subpasses.size();
        renderPassInfo->pSubpasses = subpasses.data();
        renderPassInfo->dependencyCount = (int)dependency.size();
        renderPassInfo->pDependencies = dependency.data();

        return renderPassInfo;
    }

    VkFormat engine::chooseSwapSurfaceFormat() {
        std::vector<VkSurfaceFormatKHR> availableFormats = renderer.device.getSwapChainSupport().formats;
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat.format;
            }
        }

        return availableFormats[0].format;
    }
}
