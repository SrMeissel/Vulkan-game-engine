#include "app.hpp"
#include "renderSystem.hpp"

#include <iostream>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {

    app::app() {
        loadGameObjects();
    }
    app::~app() {
        
    }

    void app::run() {
        RenderSystem renderSystem{device, renderer.getRenderPass()};
        while(!window.shouldClose()){
            glfwPollEvents();
            if(auto commandBuffer = renderer.beginFrame()) {
                renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.device());
    }

    //optional cool thing
    void app::sierpinski(std::vector<Model::Vertex> &verticies, glm::vec2 left, glm::vec2 right, glm::vec2 top, int iterations) {
        if(iterations <= 0) {
            verticies.push_back({{left}, {1.0f, 0.0f, 0.0f}});
            verticies.push_back({{right}, {0.0f, 1.0f, 0.0f}});
            verticies.push_back({{top}, {0.0f, 0.0f, 1.0f}});
        }
        else {
            auto leftTop = 0.5f*(left + top);
            auto leftRight = 0.5f*(left + right);
            auto rightTop = 0.5f*(right + top);

            sierpinski(verticies, left, leftRight, leftTop, iterations-1);
            sierpinski(verticies, leftRight, right, rightTop, iterations-1);
            sierpinski(verticies, leftTop, rightTop, top, iterations-1);
        }
    }

    void app::loadGameObjects() {
        std::vector<Model::Vertex> verticies {
            {{0.0f, -0.5f},{1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        //sierpinski(verticies, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}, 4);
        auto model = std::make_shared<Model>(device, verticies);

        auto triangle = GameObject::createGameObject();

        triangle.model = model;
        triangle.color = {0.1f, 0.8f, 0.1f};
        triangle.transform2d.translation.x = 0.2f;
        triangle.transform2d.scale = {2.0f, 0.5f};
        triangle.transform2d.rotation = 0.25F * glm::two_pi<float>(); 

        gameObjects.push_back(std::move(triangle));
    }
}