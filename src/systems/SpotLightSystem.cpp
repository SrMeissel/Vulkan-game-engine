#include "SpotLightSystem.hpp"

#include <iostream>
#include <stdexcept>

namespace engine {
    SpotLightSystem::SpotLightSystem(renderer::Device& device, renderer::RenderPass* renderPass, VkDescriptorSetLayout globalSetLayout): device{device} {
        //create shadowmap renderpass ===============================================
        VkFormat depthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        static VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.format = depthFormat;
        attachmentDescription.samples = device.msaaSamples;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        static VkAttachmentReference attachmentRef = {};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        static VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pColorAttachments = nullptr;
        subpass.pDepthStencilAttachment = &attachmentRef;

        static VkSubpassDependency dependency = {};
        dependency.srcSubpass = 0;
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage of writing to the color attachment
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Stage of reading from the attachment in the shader
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Access type for writing to the color attachment
        dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; 
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        renderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo->attachmentCount = 1;
        renderPassInfo->pAttachments = &attachmentDescription;
        renderPassInfo->subpassCount = 1;
        renderPassInfo->pSubpasses = &subpass;
        renderPassInfo->dependencyCount = 1;
        renderPassInfo->pDependencies = &dependency;

        vkCreateRenderPass(device.device(), renderPassInfo, nullptr, &shadowPass);

        //init camera buffer descriptor set ===========================================

        shadowUBO = std::make_unique<renderer::Buffer>(device, sizeof(PointLightUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        shadowUBO->map();

        lightUBO = std::make_unique<renderer::Buffer>(device, sizeof(PointLightUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        lightUBO->map();

        UBOPool = renderer::DescriptorPool::Builder(device)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3) // <==================
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
        .build();

        UBOSetLayout = renderer::DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();


        renderer::DescriptorWriter writer(*UBOSetLayout, *UBOPool);

        auto shadowBufferInfo = shadowUBO->descriptorInfo();
        writer.writeBuffer(0, &shadowBufferInfo);
        writer.build(shadowUBOSet);

        auto lightBufferInfo = lightUBO->descriptorInfo();
        writer.writeBuffer(0, &lightBufferInfo);
        writer.build(lightUBOSet);

        //create shadow Pipeline Layout ==================================================

        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ShadowPushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{UBOSetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &shadowPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }

        //create shadow Pipeline ==================================================

        renderer::PipelineConfigInfo shadowPipelineConfig{};
        renderer::Pipeline::defaultPipelineConfigInfo(shadowPipelineConfig, device);
        shadowPipelineConfig.renderPass = shadowPass;
        shadowPipelineConfig.subpass = 0;

        shadowPipelineConfig.pipelineLayout = shadowPipelineLayout;

        std::vector<std::string> files = {(std::string)SOURCE_PATH + "/shaders/shadow.vert.spv", (std::string)SOURCE_PATH + "/shaders/shadow.frag.spv"};
        std::vector<VkShaderStageFlagBits> flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        shadowPipeline = std::make_unique<renderer::Pipeline>(device, files, flags, shadowPipelineConfig);

        //create light Pipeline Layout ==================================================
        //can reuse info objects from other pipeline, i hope.

        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(LightPushConstant);

        std::vector<VkDescriptorSetLayout> lightDescriptorSetLayouts{UBOSetLayout->getDescriptorSetLayout()};

        inputSetLayout = renderer::DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
        lightDescriptorSetLayouts.push_back(inputSetLayout->getDescriptorSetLayout());


        lightSetLayout = renderer::DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
        lightDescriptorSetLayouts.push_back(lightSetLayout->getDescriptorSetLayout());

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(lightDescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = lightDescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &lightPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
        
        //create light Pipeline ==================================================

        renderer::PipelineConfigInfo lightPipelineConfig{};
        renderer::Pipeline::defaultPipelineConfigInfo(lightPipelineConfig, device);
        lightPipelineConfig.renderPass = renderPass->renderPass;
        lightPipelineConfig.subpass = 1;

        lightPipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        lightPipelineConfig.colorBlendInfo.pAttachments = &colorBlendAttachment;
        lightPipelineConfig.colorBlendInfo.attachmentCount = 1;

        lightPipelineConfig.pipelineLayout = lightPipelineLayout;

        files = {(std::string)SOURCE_PATH +"/shaders/spotLight.vert.spv", (std::string)SOURCE_PATH + "/shaders/spotLight.frag.spv"};
        flags = { VK_SHADER_STAGE_VERTEX_BIT,  VK_SHADER_STAGE_FRAGMENT_BIT};
        lightPipeline = std::make_unique<renderer::Pipeline>(device, files, flags, lightPipelineConfig);

        //create Sampler ==================================================
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        // create input attachment set ==========================================
        
        descriptors[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[0].imageView = renderPass->images[0].imageView;
        descriptors[0].sampler = VK_NULL_HANDLE;

        descriptors[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[1].imageView = renderPass->images[1].imageView;
        descriptors[1].sampler = VK_NULL_HANDLE;

        descriptors[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptors[2].imageView = renderPass->images[2].imageView;
        descriptors[2].sampler = VK_NULL_HANDLE;

        renderer::DescriptorWriter inputWriter(*inputSetLayout, *UBOPool);

        if(inputWriter.writeImages(0, descriptors.data(), 3).build(inputSet) == false)
            std::cout << "\n failed to write set \n";

    }

    void SpotLightSystem::RenderShadows(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assets, ECS::System& renderables) {
        for(auto const& Entity : entities) {
            ECS::Transform& transform = assets.GetComponent<ECS::Transform>(Entity);
            ECS::SpotLight& spotlight = assets.GetComponent<ECS::SpotLight>(Entity);
            ECS::Camera& camera = assets.GetComponent<ECS::Camera>(Entity);

            //update camera ================================================================================================

            camera.viewMatrix = setViewTarget(transform.translation, glm::vec3(0.0, -3, -3));             
            camera.projectionMatrix = setPerspectiveProjection(glm::radians(50.0f), spotlight.aspect, camera.nearPlane, camera.farPlane);
            camera.inverseViewMatrix = glm::inverse(camera.viewMatrix);

            //start shadow renderpass =============================================================================

            VkRenderPassAttachmentBeginInfo attachmentBeginInfo = {};
            attachmentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
            attachmentBeginInfo.attachmentCount = 1;
            attachmentBeginInfo.pNext = nullptr;
            attachmentBeginInfo.pAttachments = &spotlight.shadowMap.imageView;

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.pNext = &attachmentBeginInfo;
            renderPassInfo.renderPass = shadowPass;
            renderPassInfo.framebuffer = spotlight.frameBuffer;
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = {static_cast<uint32_t>(spotlight.resolution.x), static_cast<uint32_t>(spotlight.resolution.y)};
            VkClearValue clearValue = {};
            clearValue.depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearValue;
            
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                    
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f; 
            viewport.width = static_cast<float>(spotlight.resolution.x);
            viewport.height = static_cast<float>(spotlight.resolution.y);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, {static_cast<uint32_t>(spotlight.resolution.x), static_cast<uint32_t>(spotlight.resolution.y)}};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            //render shadowmap ! ==================================================================================

            shadowPipeline->bind(commandBuffer);

            PointLightUBO ubo{};
            ubo.projection = camera.projectionMatrix;
            ubo.view = camera.viewMatrix;
            ubo.inverseView = camera.inverseViewMatrix;
            ubo.farPlane = camera.farPlane;
            ubo.nearPlane = camera.nearPlane;
            shadowUBO->writeToBuffer(&ubo);
            shadowUBO->flush();

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout, 0, 1, &shadowUBOSet, 0, nullptr);

            for(auto const& meshEntity : renderables.entities) {

                //get components ==================================================

                ECS::Transform& transform = assets.GetComponent<ECS::Transform>(meshEntity);
                ECS::Renderable& mesh = assets.GetComponent<ECS::Renderable>(meshEntity);

                //push constants ==================================================

                ShadowPushConstant push{};
                push.modelMatrix = transform.mat4();
                push.normalMatrix = transform.normalMatrix();
                vkCmdPushConstants(commandBuffer, shadowPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShadowPushConstant), &push);

                //bind vertex buffer ==================================================

                VkBuffer buffers[] = {mesh.vertexBuffer->getBuffer()};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                if(mesh.hasIndexBuffer) {
                    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
                }

                //draw ==================================================

                if(mesh.hasIndexBuffer) {
                    vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
                } else{
                    vkCmdDraw(commandBuffer, mesh.vertexCount, 1, 0, 0);
                }
            }

            vkCmdEndRenderPass(commandBuffer);
        }
    }

    void SpotLightSystem::RenderLight(VkCommandBuffer commandBuffer, const ECS::Camera& viewerCamera, ECS::AssetSystem& assets) {
        lightPipeline->bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipelineLayout, 1, 1, &inputSet, 0, nullptr);

        for(const auto& Entity : entities) {
            ECS::Transform& transform = assets.GetComponent<ECS::Transform>(Entity);
            ECS::SpotLight& spotlight = assets.GetComponent<ECS::SpotLight>(Entity);
            ECS::Camera& camera = assets.GetComponent<ECS::Camera>(Entity);

            PointLightUBO ubo{};
            ubo.projection = viewerCamera.projectionMatrix;
            ubo.view = viewerCamera.viewMatrix;
            ubo.inverseView = viewerCamera.inverseViewMatrix;
            ubo.farPlane = camera.farPlane;
            ubo.nearPlane = camera.nearPlane;
            lightUBO->writeToBuffer(&ubo);
            lightUBO->flush();

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipelineLayout, 0, 1, &lightUBOSet, 0, nullptr);

            LightPushConstant push{};
            push.lightMatrix = camera.projectionMatrix * camera.viewMatrix;
            push.color = glm::vec4(spotlight.color, 1.0);
            push.position = glm::vec4(transform.translation, 1.0);
            push.intensity = spotlight.intensity;
            
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipelineLayout, 2, 1, &spotlight.descriptorSet, 0, nullptr);

            vkCmdPushConstants(commandBuffer, lightPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(LightPushConstant), &push);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        }
    }

    void SpotLightSystem::cleanup(ECS::AssetSystem& assetManager) {
        for(auto& entity : entities) {
            ECS::SpotLight& spotLight = assetManager.GetComponent<ECS::SpotLight>(entity);

            vkDestroyImageView(device.device(), spotLight.shadowMap.imageView, nullptr);
            vkDestroyImage(device.device(), spotLight.shadowMap.image, nullptr);
            vkFreeMemory(device.device(), spotLight.shadowMap.memory, nullptr);

            vkDestroyFramebuffer(device.device(), spotLight.frameBuffer, nullptr);

        }
    }
}
