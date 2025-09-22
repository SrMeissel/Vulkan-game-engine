#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/Renderer.hpp"
#include "screenshotTool.hpp"

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"

#include "systems/ScriptingSystem.hpp"
#include "systems/meshSystem.hpp"
#include "systems/MaterialSystem.hpp"
#include "systems/PointLightSystem.hpp"
#include "systems/skyboxSystem.hpp"
#include "systems/SpotLightSystem.hpp"

#include "ECS/SavedataManager.hpp"

namespace engine {
    class engine {
        public:
            static constexpr int WIDTH = 1200;
            static constexpr int HEIGHT = 800;

            engine(renderer::Renderer& renderer, ECS::AssetSystem& assetSystem);

            engine(const engine &) = delete;
            engine &operator=(const engine &) = delete;

            void updateGameState(float deltaTime, Window& window);
            void renderGameState(const ECS::Camera& target);

            ECS::Entity viewerObject; //is this even used?

            ECS::SaveDataManager* saveDataManager = nullptr;
        private:
            VkRenderPassCreateInfo* configureRenderPass();
            VkFormat chooseSwapSurfaceFormat();

            renderer::Renderer& renderer;
            ECS::AssetSystem& assetSystem;

            ScreenshotTool screenshotTool;

            MeshSystem meshSystem;
            MaterialSystem materialSystem;
            ScriptingSystem scriptingSystem;
            PointLightSystem pointLightSystem;
            SpotLightSystem spotLightSystem;
            SkyboxSystem skyboxSystem;
    };
}
