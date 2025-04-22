#pragma once

#include <../libs/stb/stb_image.h>

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Components.hpp"
#include "../bufferManager.hpp"
#include "../Utils.hpp"

namespace Importer {

    static renderer::AllocatedImage loadJPGImage(const std::string& filepath, renderer::Device& device, VkFormat format) {
        
        renderer::AllocatedImage image{};
        image.path = (std::string)SOURCE_PATH + filepath;
        
        //create image ========================================================================================
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4; // whats the 4? channels?

        if(!pixels){
            throw std::runtime_error("failed to load texture image!");
        }

        renderer::Buffer stagingBuffer{device, sizeof(pixels[0]), static_cast<uint32_t>(imageSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
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
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.image, image.memory);
        
        device.transitionImageLayout(image.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device.copyBufferToImage(stagingBuffer.getBuffer(), image.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
    
        device.transitionImageLayout(image.image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //create image view ========================================================================================
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
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

    static renderer::CubeMap loadCubeMap(const std::string filepath, std::vector<std::string> tags, renderer::Device& device, VkFormat format) {
        assert(tags.size() == 6 && "not all cube faces are filled, dumbass" );

        renderer::CubeMap cubeMap{};

        size_t index = filepath.find_last_of('.');
        std::string fileName = SOURCE_PATH + filepath.substr(0, index);
        std::string extention = filepath.substr(index);
        std::string imagePath = fileName + tags[0] + extention;

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if(!pixels) throw std::runtime_error("failed to load texture image!"); 

        VkDeviceSize imageSize = texWidth * texHeight * 4;
        renderer::Buffer stagingBuffer{device, sizeof(pixels[0]), static_cast<uint32_t>(imageSize * 6), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map(imageSize);
        stagingBuffer.writeToBuffer((void*)pixels, imageSize);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        for(int i=1; i < 6; i++) {
            imagePath = fileName + tags[i] + extention;

            pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if(!pixels) throw std::runtime_error("failed to load texture image!"); 

            stagingBuffer.map(imageSize, imageSize * i);
            stagingBuffer.writeToBuffer((void*)pixels, imageSize);
            stagingBuffer.unmap();

            stbi_image_free(pixels);
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texWidth;
        imageInfo.extent.height = texHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6; // <------------------------
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;    
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;    
        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeMap.image, cubeMap.memory);

        //this is a custom version of the Device::transitionImageLayout function =================================================================
            VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.image = cubeMap.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 6;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

            vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );
            device.endSingleTimeCommands(commandBuffer);
        // ==============================================================================================================================

        device.copyBufferToImage(stagingBuffer.getBuffer(), cubeMap.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);

        // another unique copy of the function ==========================================================================================
            VkCommandBuffer commandBuffer2 = device.beginSingleTimeCommands();

            //VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.image = cubeMap.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 6;

            //VkPipelineStageFlags sourceStage;
            //VkPipelineStageFlags destinationStage;

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            vkCmdPipelineBarrier(
            commandBuffer2,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

            device.endSingleTimeCommands(commandBuffer2);
        // ================================================================================================================================

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubeMap.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &cubeMap.imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return cubeMap;

    }

    static ECS::SkyBox loadSkyBox(const std::string filepath, std::vector<std::string> tags, renderer::Device& device, VkSampler sampler , std::unique_ptr<renderer::DescriptorSetLayout>& skyboxSetLayout) {
        ECS::SkyBox skybox;
        skybox.Path = filepath;
        skybox.tags = tags;

        skybox.skyBoxImage = loadCubeMap(filepath, tags, device, VK_FORMAT_R8G8B8A8_SRGB);
        skybox.descriptorPool = renderer::DescriptorPool::Builder(device).setMaxSets(3)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .build();

        renderer::DescriptorWriter writer(*skyboxSetLayout, *skybox.descriptorPool);
        skybox.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        skybox.imageInfo.imageView = skybox.skyBoxImage.imageView;
        skybox.imageInfo.sampler = sampler;
        if(writer.writeImage(0, &skybox.imageInfo, 1).build(skybox.descriptorSet) == false) std::cout << "\n failed to write set \n";

        return skybox;
    }

    static ECS::Material loadMaterial(std::string albedoPath, std::string normalPath, renderer::Device& device, VkSampler sampler, std::unique_ptr<renderer::DescriptorSetLayout>& materialSetLayout) {
            ECS::Material material{};

            material.albedo = Importer::loadJPGImage(albedoPath, device, VK_FORMAT_R8G8B8A8_SRGB);
            material.normal = Importer::loadJPGImage(normalPath, device, VK_FORMAT_R8G8B8A8_UNORM);

            material.descriptorPool = renderer::DescriptorPool::Builder(device).setMaxSets(3)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
            .build();

            material.albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            material.albedoImageInfo.imageView = material.albedo.imageView;
            material.albedoImageInfo.sampler = sampler;

            material.normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            material.normalImageInfo.imageView = material.normal.imageView;
            material.normalImageInfo.sampler = sampler;

            material.samplerInfo.sampler = sampler;

            renderer::DescriptorWriter writer(*materialSetLayout, *material.descriptorPool);

            if(writer.writeImage(0, &material.samplerInfo, 1).writeImage(1,&material.albedoImageInfo, 1).writeImage(2,&material.normalImageInfo, 1).build(material.descriptorSet) == false) std::cout << "\n failed to write set \n";

            return material;
        }

    static ECS::Renderable loadMesh(std::string filepath, renderer::Device& device) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile((std::string)SOURCE_PATH + filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
        }

        ECS::Renderable renderable{};
        renderable.Path = filepath;
        
        std::vector<renderer::Vertex> vertices;
        std::vector<uint32_t> indices;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[i];
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                renderer::Vertex vertex{};
                vertex.position = {mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z};
                if(mesh->HasVertexColors(0)) {
                    vertex.color = {mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b};
                } else {
                    vertex.color = {1.0f, 1.0f, 1.0f};
                }
                if (mesh->HasNormals()) {
                    vertex.normal = {mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};
                }
                if (mesh->HasTextureCoords(0)) {
                    vertex.uv = {mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};
                }
                if (mesh->HasTangentsAndBitangents()) {
                    vertex.tangent = {mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z};
                    vertex.biTangent = {mesh->mBitangents[j].x, mesh->mBitangents[j].y, mesh->mBitangents[j].z};
                }

                vertices.push_back(vertex);
            }
            for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
                const aiFace& face = mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; k++) {
                    indices.push_back(face.mIndices[k]);
                }
            }
        }

        //create vertex buffer ========================================================================================
        renderable.vertexCount = static_cast<uint32_t>(vertices.size());
        assert(renderable.vertexCount >= 3 && "VertexCount must be at least 3!");
        VkDeviceSize bufferSize = sizeof(vertices[0])*renderable.vertexCount;

        uint32_t vertexSize = sizeof(vertices[0]);

        renderer::Buffer stagingBuffer{device, vertexSize, renderable.vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        renderable.vertexBuffer = std::make_unique<renderer::Buffer>(device, vertexSize, renderable.vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 
        device.copyBuffer(stagingBuffer.getBuffer(), renderable.vertexBuffer->getBuffer(), bufferSize);

        //create index buffer ========================================================================================
        renderable.indexCount = static_cast<uint32_t>(indices.size());
        renderable.hasIndexBuffer = renderable.indexCount > 0;

        if(renderable.hasIndexBuffer) {
            VkDeviceSize bufferSize = sizeof(indices[0])*renderable.indexCount; 
            uint32_t indexSize = sizeof(indices[0]);
            renderer::Buffer stagingBuffer{device, indexSize, renderable.indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

            stagingBuffer.map();
            stagingBuffer.writeToBuffer((void*)indices.data());

            renderable.indexBuffer = std::make_unique<renderer::Buffer>(device, indexSize, renderable.indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                device.copyBuffer(stagingBuffer.getBuffer(), renderable.indexBuffer->getBuffer(), bufferSize);
        }
        return renderable;
    }


}