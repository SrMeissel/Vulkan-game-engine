#pragma once

#include <tiny_obj_loader.h>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

#include "ECS/Components.hpp"
#include "bufferManager.hpp"
#include "Utils.hpp"

namespace Importer {
    
    // returns renderable component
    ECS::Renderable loadOBJmodel(const std::string& filepath, engine::Device& device) {

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

                //implementing index buffer to reduce memory size
                if(uniqueVertices.count(vertex) == 0){
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        
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
}