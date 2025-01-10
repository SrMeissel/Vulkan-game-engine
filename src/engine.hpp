#pragma once

#include "Pipeline/windowManager.hpp"
#include "Pipeline/deviceManager.hpp"
#include "Pipeline/Renderer.hpp"
#include "descriptorManager.hpp"
#include "screenshotTool.hpp"
#include "Tools/SceneEditor.hpp"

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

            void run();
        private:
            VkRenderPassCreateInfo* configureRenderPass();
            VkFormat chooseSwapSurfaceFormat();

            renderer::Renderer& renderer;

            ScreenshotTool screenshotTool;

            ECS::AssetSystem& assetSystem;
    };
}