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

//windows BULLSHIT!!!!!!
#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

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

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* renderable = doc.NewElement("Renderable");
            renderable->SetText(Path.c_str());
            return renderable;
        }
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

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* material = doc.NewElement("Material");
            material->SetAttribute("albedoPath", albedo.c_str());
            material->SetAttribute("normalPath", normal.c_str());
            return material;
        }
    };

    struct SkyBox : public Component {
        // https://satellitnorden.wordpress.com/2018/01/23/vulkan-adventures-cube-map-tutorial/
        SkyBox() = default;

        std::vector<std::string> tags;
        ImageResource skyBoxImage{};

        VkDescriptorImageInfo imageInfo;
        std::shared_ptr<renderer::DescriptorPool> descriptorPool;
        VkDescriptorSet descriptorSet;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* skyBox = doc.NewElement("SkyBox");
            skyBox->SetText(skyBoxImage.c_str());
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
        SpotLight(renderer::Device& device, Window& window, glm::vec3 color, float intensity, glm::vec2 resolution, VkRenderPass pass, VkSampler sampler, std::unique_ptr<renderer::DescriptorSetLayout>& setLayout) : 
        color{color}, intensity{intensity}, resolution{resolution}, aspect{static_cast<float>(resolution.x) / static_cast<float>(resolution.y)} {

            shadowMap.device = device.device();
            // create images ===========================================================================

            VkFormat depthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

            //create framebuffer ===========================================

            VkFramebufferAttachmentImageInfo attachmentInfo;
            attachmentInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.flags = 0;
            attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            attachmentInfo.width = static_cast<uint32_t>(resolution.x);
            attachmentInfo.height = static_cast<uint32_t>(resolution.y);
            attachmentInfo.layerCount = 1;
            attachmentInfo.viewFormatCount = 1;
            attachmentInfo.pViewFormats = &depthFormat;

            VkFramebufferAttachmentsCreateInfo attachmentsCreateInfo = {};
            attachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
            attachmentsCreateInfo.pNext = nullptr;
            attachmentsCreateInfo.attachmentImageInfoCount = 1;
            attachmentsCreateInfo.pAttachmentImageInfos = &attachmentInfo;

            static VkFramebufferCreateInfo framebufferInfo;
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = &attachmentsCreateInfo;
            framebufferInfo.renderPass = pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = nullptr;
            framebufferInfo.width = static_cast<uint32_t>(resolution.x);
            framebufferInfo.height =  static_cast<uint32_t>(resolution.y);
            framebufferInfo.layers = 1;
            framebufferInfo.flags |= VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
            framebufferInfo.pAttachments = nullptr;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
            //create descriptor =======================================================================================
            descriptorPool = renderer::DescriptorPool::Builder(device).setMaxSets(2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
            .build();

            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo.imageView = shadowMap.imageView;
            descriptorImageInfo.sampler = sampler;

            samplerInfo.sampler = sampler;

            renderer::DescriptorWriter writer(*setLayout, *descriptorPool);
            if(writer.writeImage(0, &samplerInfo, 1).writeImage(1,&descriptorImageInfo, 1).build(descriptorSet) == false) std::cout << "\n failed to write spotlight component set \n";

        }

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
        
        std::string name{}; //TODO: implement for editor use only 

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

        //TODO: Ponder why i used "this->" ???
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
        Camera() = default;
        Camera(float near, float far) : nearPlane{near}, farPlane{far} {}
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};
        float nearPlane;
        float farPlane;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* camera = doc.NewElement("Camera");
            camera->SetAttribute("near", this->nearPlane);
            camera->SetAttribute("far", this->farPlane);
            return camera;
        }
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
	
        }

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* script = doc.NewElement("Script");
            std::cout << "Saving Script: " << className << std::endl;
            script->SetText(className.c_str());
            return script;
        }

    };

}
