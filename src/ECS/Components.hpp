#pragma once

#include "bufferManager.hpp"
#include "Utils.hpp"
#include "../../libs/tinyXML/tinyxml2.h"
#include <descriptorManager.hpp>

#include <glm/glm.hpp>
#include <string>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace ECS {

    using ImageResource = std::string;

    struct Component {
        virtual ~Component() = default;
        virtual tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) = 0;
    };

    struct Renderable : public Component {
        Renderable() = default;

        std::string Path;

        std::shared_ptr<renderer::Buffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::shared_ptr<renderer::Buffer> indexBuffer;
        uint32_t indexCount;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override; 
    };

    struct Material : public Component {
        Material() = default;

        ImageResource albedo;
        ImageResource normal;

        VkDescriptorImageInfo albedoImageInfo;
        VkDescriptorImageInfo normalImageInfo;

        VkDescriptorImageInfo samplerInfo;

        std::shared_ptr<renderer::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct SkyBox : public Component {
        // https://satellitnorden.wordpress.com/2018/01/23/vulkan-adventures-cube-map-tutorial/
        SkyBox() = default;

        std::vector<std::string> tags;
        ImageResource skyBoxImage{};

        VkDescriptorImageInfo imageInfo;
        std::shared_ptr<renderer::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct PointLight : public Component {  
        PointLight() = default;
        PointLight(glm::vec3 color, float intensity, float radius) : color(color), intensity(intensity), radius(radius) {}

        glm::vec3 color;
        float radius;
        float intensity;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct SpotLight : public Component {
        SpotLight() = default;
        SpotLight(renderer::Device& device, Window& window, glm::vec3 color, float intensity, glm::vec2 resolution, VkRenderPass pass, VkSampler sampler, std::unique_ptr<renderer::DescriptorSetLayout>& setLayout);

        glm::vec3 color;
        float intensity;
        glm::vec2 resolution;
        float aspect;

        renderer::AllocatedImage shadowMap{};
        //ImageResource shadowMap;

        VkFramebuffer frameBuffer;

        VkDescriptorImageInfo descriptorImageInfo;
        std::shared_ptr<renderer::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;
        VkDescriptorImageInfo samplerInfo;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct Transform : public Component {
        Transform() = default;
        Transform(glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation)
            : translation(translation), scale(scale), rotation(rotation) {}

        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};
        
        std::string name{}; //TODO: implement for editor use only 

        glm::mat4 mat4(); 
        glm::mat3 normalMatrix();

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct Camera : public Component {
        Camera() = default;
        Camera(float nearPlane, float farPlane) : nearPlane{nearPlane}, farPlane{farPlane} {}

        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};
        float nearPlane;
        float farPlane;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };

    struct Script : public Component {
        std::string className;
        MonoClass* scriptClass;
        MonoObject* scriptObject;
        MonoClass* objectClass;

        #ifdef _WIN32
            #define CALLING_CONVENTION __stdcall
        #else
            #define CALLING_CONVENTION
        #endif

        typedef void (CALLING_CONVENTION *Update)(MonoObject*, MonoException**);
        MonoMethod* updateMethod;
        Update update;

        typedef void (CALLING_CONVENTION *LoadTransform)(MonoObject*, float, float, float, float, float, float, MonoException**);
        MonoMethod* loadTransformMethod;
        LoadTransform loadTransform;

        MonoClass* parentClass;
        MonoImage* image;

        Script() = default;

        Script(const char* name, MonoAssembly* assembly, MonoDomain* appDomain);

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override;
    };
}