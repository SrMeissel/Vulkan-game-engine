#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/Renderer.hpp"
#include "descriptorManager.hpp"
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

#include <memory>
#include <vector>

namespace engine {
    class engine {
        public:
            static constexpr int WIDTH = 1200;
            static constexpr int HEIGHT = 800;

            engine(renderer::Renderer& renderer, ECS::AssetSystem& assetSystem);
            ~engine();

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

            std::shared_ptr<MeshSystem> meshSystem;
            std::shared_ptr<MaterialSystem> materialSystem;
            std::shared_ptr<ScriptingSystem> scriptingSystem;
            std::shared_ptr<PointLightSystem> pointLightSystem;
            std::shared_ptr<SpotLightSystem> spotLightSystem;
            std::shared_ptr<SkyboxSystem> skyboxSystem;
            //I need a list of all renderable objects for shadows. This makes me want to detach the entity list from systems, It would be a lot more simple.
            std::shared_ptr<Renderables> renderables;

            
            // renderer::RenderPass* scenePass;
    };
}
