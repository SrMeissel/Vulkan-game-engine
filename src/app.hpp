#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "gameObject.hpp"
#include "Pipeline/Renderer.hpp"
#include "descriptorManager.hpp"
#include "screenshotTool.hpp"
#include "textureManager.hpp"
#include "Components/Physics/PxSimulation.hpp"

#include <memory>
#include <vector>

namespace engine {
    class app {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            app();
            ~app();

            app(const app &) = delete;
            app &operator=(const app &) = delete;

            void run();
        private:
            void loadGameObjects();
            void initilizeObject(GameObject& object, glm::vec3 position, glm::vec3 scale, std::string modelFile, int textureIndex = 0);
            void loadTextures();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            Renderer renderer{window, device};
            PhysicsSimulation physicsSimulation{};


            std::unique_ptr<DescriptorPool> globalPool;
            GameObject::map gameObjects;

            TextureManager textureManager{device}; 
            Texture loadedTextures[3];

            ScreenshotTool screenshotTool;
    };
}