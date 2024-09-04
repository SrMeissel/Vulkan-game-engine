#pragma once

#include "Pipeline/RenderPass.hpp"
#include "bufferManager.hpp"
#include "Utils.hpp"
#include "../../libs/tinyXML/tinyxml2.h"
#include <descriptorManager.hpp>

#include <memory>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace ECS {

    struct Component {
        virtual ~Component() = default;
        virtual tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) = 0; // <======
    };

    struct Renderable : public Component {
        Renderable() = default;

        std::string Path;

        std::shared_ptr<engine::Buffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::shared_ptr<engine::Buffer> indexBuffer;
        uint32_t indexCount;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* renderable = doc.NewElement("Renderable");
            renderable->SetText(Path.c_str());
            return renderable;
        }
    };

    struct Material : public Component {
        Material() = default;

        engine::AllocatedImage albedo;
        engine::AllocatedImage normal;

        VkDescriptorImageInfo albedoImageInfo;
        VkDescriptorImageInfo normalImageInfo;

        VkDescriptorImageInfo samplerInfo;

        std::shared_ptr<engine::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* material = doc.NewElement("Material");
            material->SetAttribute("albedoPath", albedo.path.c_str());
            material->SetAttribute("normalPath", normal.path.c_str());
            return material;
        }
    };

    struct SkyBox : public Component {
        // https://satellitnorden.wordpress.com/2018/01/23/vulkan-adventures-cube-map-tutorial/
        SkyBox() = default;
        SkyBox(std::string path) : Path(path) {}

        std::string Path;
        std::vector<std::string> tags;

        engine::CubeMap skyBoxImage;

        VkDescriptorImageInfo imageInfo;
        std::shared_ptr<engine::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* skyBox = doc.NewElement("SkyBox");
            skyBox->SetText(Path.c_str());
            skyBox->SetAttribute("Right", tags[0].c_str());
            skyBox->SetAttribute("Left", tags[1].c_str());
            skyBox->SetAttribute("Up", tags[2].c_str());
            skyBox->SetAttribute("Down", tags[3].c_str());
            skyBox->SetAttribute("Front", tags[4].c_str());
            skyBox->SetAttribute("Back", tags[5].c_str());
            return skyBox;
        }
    };

    struct PointLight : public Component {
        PointLight() = default;
        PointLight(glm::vec3 color, float intensity, float radius) : color(color), intensity(intensity), radius(radius) {}

        glm::vec3 color;
        float radius;
        float intensity;

        engine::AllocatedImage shadowMap;
        VkFramebuffer shadowMapFrameBuffer;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* pointLight = doc.NewElement("PointLight");
            pointLight->SetAttribute("R", color.r);
            pointLight->SetAttribute("G", color.g);
            pointLight->SetAttribute("B", color.b);
            pointLight->SetAttribute("intensity", intensity);
            pointLight->SetAttribute("radius", radius);
            return pointLight;
        }
    };

    struct SpotLight : public Component {
        SpotLight() = default;
        SpotLight(engine::Device& device, engine::Window& window, glm::vec3 color, float intensity, glm::vec2 resolution) : 
        color{color}, intensity{intensity}, resolution{resolution} {

            // create images ===========================================================================

            VkFormat depthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = static_cast<uint32_t>(resolution.x);
            imageInfo.extent.height = static_cast<uint32_t>(resolution.y);
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = device.msaaSamples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowMap.image, shadowMap.memory);

            //create image view ===========================================================
            
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = shadowMap.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; 
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(device.device(), &viewInfo, nullptr, &shadowMap.imageView);

            std::vector<engine::AllocatedImage> images{shadowMap};

            //define renderpass ===============================================

            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.format = depthFormat;
            attachmentDescription.samples = device.msaaSamples;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentReference attachmentRef = {};
            attachmentRef.attachment = 0;
            attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 0;
            subpass.pColorAttachments = nullptr;
            subpass.pDepthStencilAttachment = &attachmentRef;

            VkSubpassDependency dependency = {};
            dependency.srcSubpass = 0;
            dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Stage of writing to the color attachment
            dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Stage of reading from the attachment in the shader
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Access type for writing to the color attachment
            dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; 
            dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo* renderPassInfo = new VkRenderPassCreateInfo();
            renderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo->attachmentCount = 1;
            renderPassInfo->pAttachments = &attachmentDescription;
            renderPassInfo->subpassCount = 1;
            renderPassInfo->pSubpasses = &subpass;
            renderPassInfo->dependencyCount = 1;
            renderPassInfo->pDependencies = &dependency;

            //final ====================================================================

            shadowPass = std::make_shared<engine::RenderPass>(device, window, renderPassInfo, images, false, VkExtent2D{800, 600});

        }

        glm::vec3 color;
        float intensity;

        glm::vec2 resolution;
        
        engine::AllocatedImage shadowMap{};
        std::shared_ptr<engine::RenderPass> shadowPass;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            return nullptr;
        }
    };

    struct Transform : public Component {
        Transform() = default;
        Transform(glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation)
            : translation(translation), scale(scale), rotation(rotation) {}

        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        // https://www.youtube.com/watch?v=0X_kRtyVzm4&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=14&t=289s
        glm::mat4 mat4() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);

            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2), // <=== position 3,3
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
        }
        glm::mat3 normalMatrix() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            const glm::vec3 invScale = 1.0f / scale;

            return glm::mat3{
                {
                    invScale.x * (c1 * c3 + s1 * s2 * s3),
                    invScale.x * (c2 * s3),
                    invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                    invScale.y * (c3 * s1 * s2 - c1 * s3),
                    invScale.y * (c2 * c3),
                    invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                    invScale.z * (c2 * s1),
                    invScale.z * (-s2),
                    invScale.z * (c1 * c2),
                }
            };
        }

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* transform = doc.NewElement("Transform");
            
            tinyxml2::XMLElement* translation = doc.NewElement("Translation");
            translation->SetAttribute("x", this->translation.x);
            translation->SetAttribute("y", this->translation.y);
            translation->SetAttribute("z", this->translation.z);
            transform->InsertEndChild(translation);

            tinyxml2::XMLElement* rotation = doc.NewElement("Rotation");
            rotation->SetAttribute("x", this->rotation.x);
            rotation->SetAttribute("y", this->rotation.y);
            rotation->SetAttribute("z", this->rotation.z);
            transform->InsertEndChild(rotation);

            tinyxml2::XMLElement* scale = doc.NewElement("Scale");
            scale->SetAttribute("x", this->scale.x);
            scale->SetAttribute("y", this->scale.y);
            scale->SetAttribute("z", this->scale.z);
            transform->InsertEndChild(scale);

            return transform;
        }
    };

    struct Camera : public Component {
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            return doc.NewElement("Camera");
        }
    };

    struct Script : public Component {
        std::string className;
        MonoClass* scriptClass;
        MonoObject* scriptObject;
        MonoClass* objectClass;

        typedef void (__stdcall *Update)(MonoObject*, MonoException**);
        MonoMethod* updateMethod;
        Update update;

        typedef void (__stdcall *LoadTransform)(MonoObject*, float, float, float, float, float, float, MonoException**);
        MonoMethod* loadTransformMethod;
        LoadTransform loadTransform;

        MonoClass* parentClass;
        MonoImage* image;

        Script() {
            scriptClass = nullptr;
            scriptObject = nullptr;
            objectClass = nullptr;
        }

        Script(const char* name, MonoAssembly* assembly, MonoDomain* appDomain) {
            className = name;
            image = mono_assembly_get_image(assembly);
            scriptClass = mono_class_from_name(image, "", name);
            if(scriptClass == nullptr) std::cout << "Failed to get class!" << std::endl;

            scriptObject = mono_object_new(appDomain, scriptClass);
            if(scriptObject == nullptr) std::cout << "Failed to create object!" << std::endl;

            mono_runtime_object_init(scriptObject); // constructor

            objectClass = mono_object_get_class(scriptObject);
            parentClass = mono_class_get_parent(objectClass);

            loadTransformMethod = mono_class_get_method_from_name(parentClass, "loadTransform", 6);
            loadTransform = (LoadTransform)mono_method_get_unmanaged_thunk(loadTransformMethod);

            updateMethod = mono_class_get_method_from_name(objectClass, "update", 0);
            update = (Update)mono_method_get_unmanaged_thunk(updateMethod);

            std::cout << "Script Created!" << std::endl;    

        }

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* script = doc.NewElement("Script");
            std::cout << "Saving Script: " << className << std::endl;
            script->SetText(className.c_str());
            return script;
        }

    };

}