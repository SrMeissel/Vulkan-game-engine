#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "gameObject.hpp"
#include "Pipeline/Renderer.hpp"
#include "descriptorManager.hpp"
#include "screenshotTool.hpp"
#include "Components/textureManager.hpp"
#include "Components/Physics/PxSimulation.hpp"
#include "Tools/SceneEditor.hpp"

#include "ECS/AssetManager.hpp"

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
            void initilizeObject(GameObject& object, glm::vec3 position, glm::vec3 scale, std::string modelFile, char* textureFile);
            VkRenderPassCreateInfo* configureRenderPass();
            VkFormat chooseSwapSurfaceFormat();

            Window window{WIDTH, HEIGHT, "Hello there"};
            Device device{window};
            Renderer renderer{window, device};

            SceneEditor sceneEditor{device, window, renderer};

            PhysicsSimulation physicsSimulation{};
            TextureManager textureManager{device}; 
            ScreenshotTool screenshotTool;

            std::shared_ptr<DescriptorPool> globalPool;
            
            GameObject::map gameObjects;

            ECS::AssetSystem assetSystem;
            void doECSThings(); 
    };
}