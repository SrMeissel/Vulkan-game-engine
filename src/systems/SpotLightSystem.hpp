// https://blogs.igalia.com/itoral/2017/07/30/working-with-lights-and-shadows-part-ii-the-shadow-map/
// https://stackoverflow.com/questions/9660959/how-to-use-a-single-shadow-map-for-multiple-point-light-sources
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

// https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf

#pragma once

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"
#include "Pipeline/pipeline.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/Renderer.hpp"

#include <glm/glm.hpp>

namespace engine {
    class SpotLightSystem {
    public:
        SpotLightSystem(renderer::Renderer& renderer, ECS::AssetSystem& assetManager);
        ~SpotLightSystem() { 
            vkDestroyPipelineLayout(renderer.device.device(), shadowPipelineLayout, nullptr); 
            vkDestroyPipelineLayout(renderer.device.device(), lightPipelineLayout, nullptr);
            vkDestroyRenderPass(renderer.device.device(), shadowPass, nullptr);
        
            vkDestroySampler(renderer.device.device(), sampler, nullptr);    
        }

        void RenderShadows(VkCommandBuffer commandBuffer, VkDescriptorSet& globalUBOSet, ECS::AssetSystem& assetManagers);
        void RenderLight(VkCommandBuffer commandBuffer, const ECS::Camera& viewerCamera, ECS::AssetSystem& assetManager);

        VkRenderPass getRenderPass() {return shadowPass; }
        VkSampler& getSampler() { return sampler; }        
        std::unique_ptr<renderer::DescriptorSetLayout>& getSetLayout() { return lightSetLayout; }

        const std::string systemName{"SpotLight"};
        const std::string renderablesSystemName{"Renderables"};
    private:
        struct ShadowPushConstant {
            glm::mat4 modelMatrix{1.f};
            glm::mat4 normalMatrix{1.f};
        };
        struct LightPushConstant {
            glm::mat4 lightMatrix;
            glm::vec4 position;
            glm::vec4 color;
            float intensity;
            
        };
        struct PointLightUBO {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::mat4 inverseView{1.0f};
            float nearPlane;
            float farPlane;
        };

        void recreateDescriptorSets();

        renderer::Renderer& renderer;

        std::shared_ptr<renderer::DescriptorPool> UBOPool;
        std::unique_ptr<renderer::DescriptorSetLayout> UBOSetLayout;

        // vulkan things required for the Shadow RenderPass ================================================
        VkRenderPassCreateInfo* renderPassInfo = new VkRenderPassCreateInfo();
        VkRenderPass shadowPass;
        std::unique_ptr<renderer::Buffer> shadowUBO;
        VkDescriptorSet shadowUBOSet;
        std::unique_ptr<renderer::Pipeline> shadowPipeline;
        VkPipelineLayout shadowPipelineLayout;


        // vulkan things required by the light RenderPass ==============================================
        std::unique_ptr<renderer::Buffer> lightUBO;
        VkDescriptorSet lightUBOSet;
        std::unique_ptr<renderer::DescriptorSetLayout> lightSetLayout; // <= to spotLight component
        std::unique_ptr<renderer::Pipeline> lightPipeline;
        VkPipelineLayout lightPipelineLayout;
        VkSampler sampler;

        std::unique_ptr<renderer::DescriptorSetLayout> inputSetLayout;
        std::array<VkDescriptorImageInfo, 3> descriptors{};
        VkDescriptorSet inputSet;

        std::shared_ptr<ECS::System> entities;
        std::shared_ptr<ECS::System> renderables;
    };

}