#pragma once

#include "bufferManager.hpp"
#include "Utils.hpp"
#include "../../libs/tinyXML/tinyxml2.h"
#include "../Importer.hpp"
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
        
        Renderable(std::string filepath, engine::Device& device) {
                
            // Load OBJ file =========================================================================
            std::vector<engine::Vertex> vertices{};
            std::vector<uint32_t> indices{};

            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;


            if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())){
                throw std::runtime_error(warn + err);
            }
            vertices.clear();
            indices.clear();

            std::unordered_map<engine::Vertex, uint32_t> uniqueVertices{};

            for(const auto& shape: shapes) {
                for(const auto& index : shape.mesh.indices){
                    engine::Vertex vertex{};
                    if(index.vertex_index >= 0){
                        vertex.position = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                        auto colorIndex = 3 * index.vertex_index + 2;

                        vertex.color = {attrib.colors[3 * index.vertex_index + 0], attrib.colors[3 * index.vertex_index + 1], attrib.colors[3 * index.vertex_index + 2]};

                    }
                    if(index.normal_index >= 0){
                        vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                    }
                    if(index.texcoord_index >= 0){
                        vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0], attrib.texcoords[2 * index.texcoord_index + 1]};
                    }

                    if(uniqueVertices.count(vertex) == 0){
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                }
            }

        //here is where I would calculate the tangents, maybe
        //copilot copied from https://learnopengl.com/Advanced-Lighting/Normal-Mapping
        // I want to get lighting working before I implement this       P.S. I did
        for(int i = 0; i < indices.size(); i+=3){
            engine::Vertex& v0 = vertices[indices[i]];
            engine::Vertex& v1 = vertices[indices[i+1]];
            engine::Vertex& v2 = vertices[indices[i+2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;

            glm::vec2 deltaUV1 = v1.uv - v0.uv;
            glm::vec2 deltaUV2 = v2.uv - v0.uv;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;

            glm::vec3 biTangent;
            biTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            biTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            biTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            v0.biTangent += biTangent;
            v1.biTangent += biTangent;
            v2.biTangent += biTangent;
        }

            Path = filepath;
            // create vertex buffer ========================================================================================
            std::cout << "Vertex Count: " << vertices.size() << "\n";

            vertexCount = static_cast<uint32_t>(vertices.size());
            assert(vertexCount >= 3 && "VertexCount must be at least 3!");
            VkDeviceSize bufferSize = sizeof(vertices[0])*vertexCount;

            uint32_t vertexSize = sizeof(vertices[0]);

            engine::Buffer stagingBuffer{device, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

            stagingBuffer.map();
            stagingBuffer.writeToBuffer((void *)vertices.data());

            vertexBuffer = std::make_unique<engine::Buffer>(device, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 
            device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

            // create index buffer ========================================================================================
            std::cout << "Index Count: " << indices.size() << "\n";

            indexCount = static_cast<uint32_t>(indices.size());
            hasIndexBuffer = indexCount > 0;
            if(hasIndexBuffer) {
                VkDeviceSize bufferSize = sizeof(indices[0])*indexCount; 
                uint32_t indexSize = sizeof(indices[0]);

                engine::Buffer stagingBuffer{device, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

                stagingBuffer.map();
                stagingBuffer.writeToBuffer((void*)indices.data());

                indexBuffer = std::make_unique<engine::Buffer>(device, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
            }

        }

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
        Material(std::string albedoPath, std::string normalPath, engine::Device& device, VkSampler sampler, std::unique_ptr<engine::DescriptorSetLayout>& materialSetLayout) {
            albedo = Importer::loadJPGImage(albedoPath, device);
            normal = Importer::loadJPGImage(normalPath, device);

            descriptorPool = engine::DescriptorPool::Builder(device).setMaxSets(3)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
            .build();

            albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            albedoImageInfo.imageView = albedo.imageView;
            albedoImageInfo.sampler = sampler;

            normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalImageInfo.imageView = normal.imageView;
            normalImageInfo.sampler = sampler;

            samplerInfo.sampler = sampler;

            engine::DescriptorWriter writer(*materialSetLayout, *descriptorPool);

             if(writer.writeImage(0, &samplerInfo, 1).writeImage(1,&albedoImageInfo, 1).writeImage(2,&normalImageInfo, 1).build(descriptorSet) == false)
            std::cout << "\n failed to write set \n";


        }

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

    //this seems, too simple
    struct PointLight : public Component{
        PointLight() = default;
        PointLight(glm::vec3 color) : color(color) {}

        glm::vec3 color;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* pointLight = doc.NewElement("PointLight");
            pointLight->SetAttribute("R", color.r);
            pointLight->SetAttribute("G", color.g);
            pointLight->SetAttribute("B", color.b);
            return pointLight;
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

    struct Script : public Component {
        const char* className;
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

        // I might want to find a way to make the assembly and domain accessible differently.
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
            script->SetText(className);
            return script;
        }

    };

}