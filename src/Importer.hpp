#pragma once

#include <tiny_obj_loader.h>
#include <../libs/stb/stb_image.h>

#include <stdexcept>
#include <iostream>
#include <unordered_map>

#include "ECS/Components.hpp"
#include "bufferManager.hpp"
#include "Utils.hpp"

namespace Importer {
    
    // returns renderable component
    static ECS::Renderable loadOBJmodel(const std::string& filepath, engine::Device& device) {

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
        //copilot copied from https://learnopengl.com/Advanced-Lighting/Normal-Mapping, apparently\
        // I want to get lighting working before I implement this
        // for(int i = 0; i < indices.size(); i+=3){
        //     engine::Vertex& v0 = vertices[indices[i]];
        //     engine::Vertex& v1 = vertices[indices[i+1]];
        //     engine::Vertex& v2 = vertices[indices[i+2]];

        //     glm::vec3 edge1 = v1.position - v0.position;
        //     glm::vec3 edge2 = v2.position - v0.position;

        //     glm::vec2 deltaUV1 = v1.uv - v0.uv;
        //     glm::vec2 deltaUV2 = v2.uv - v0.uv;

        //     float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        //     glm::vec3 tangent;
        //     tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        //     tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        //     tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        //     v0.tangent += tangent;
        //     v1.tangent += tangent;
        //     v2.tangent += tangent;
        // }


        ECS::Renderable renderable{};
        renderable.Path = filepath;
        // create vertex buffer ========================================================================================
        std::cout << "Vertex Count: " << vertices.size() << "\n";

        renderable.vertexCount = static_cast<uint32_t>(vertices.size());
        assert(renderable.vertexCount >= 3 && "VertexCount must be at least 3!");
        VkDeviceSize bufferSize = sizeof(vertices[0])*renderable.vertexCount;

        uint32_t vertexSize = sizeof(vertices[0]);

        engine::Buffer stagingBuffer{device, vertexSize, renderable.vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        renderable.vertexBuffer = std::make_unique<engine::Buffer>(device, vertexSize, renderable.vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 
        device.copyBuffer(stagingBuffer.getBuffer(), renderable.vertexBuffer->getBuffer(), bufferSize);

        // create index buffer ========================================================================================
        std::cout << "Index Count: " << indices.size() << "\n";

        renderable.indexCount = static_cast<uint32_t>(indices.size());
        renderable.hasIndexBuffer = renderable.indexCount > 0;
        if(renderable.hasIndexBuffer) {
            VkDeviceSize bufferSize = sizeof(indices[0])*renderable.indexCount; 
            uint32_t indexSize = sizeof(indices[0]);

            engine::Buffer stagingBuffer{device, indexSize, renderable.indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

            stagingBuffer.map();
            stagingBuffer.writeToBuffer((void*)indices.data());

            renderable.indexBuffer = std::make_unique<engine::Buffer>(device, indexSize, renderable.indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            device.copyBuffer(stagingBuffer.getBuffer(), renderable.indexBuffer->getBuffer(), bufferSize);
        }

        //========================================================================================
        return renderable;
    };

    engine::AllocatedImage loadJPGImage(const std::string& filepath, engine::Device& device) {
        
        engine::AllocatedImage image{};
        
        //create image ========================================================================================
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4; // whats the 4? channels?

        if(!pixels){
            throw std::runtime_error("failed to load texture image!");
        }

        engine::Buffer stagingBuffer{device, sizeof(pixels[0]), static_cast<uint32_t>(imageSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)pixels);

        stbi_image_free(pixels); // the pixels are now in the staging buffer so we can free the memory

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texWidth;
        imageInfo.extent.height = texHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
        
        device.transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device.copyBufferToImage(stagingBuffer.getBuffer(), image.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
    
        device.transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //create image view ========================================================================================
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &image.imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        // ========================================================================================
        return image;
    };
}