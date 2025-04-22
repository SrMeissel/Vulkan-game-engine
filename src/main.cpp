#include "engine.hpp"
#include "Pipeline/windowManager.hpp"
#include "Pipeline/Renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <chrono>

#ifndef SOURCE_PATH
#define SOURCE_PATH ""
#endif

#ifndef CONSOLE
    #include <windows.h>
#endif // CONSOLE

#ifndef EDITOR

    int main(int argc, char** argv) {
	std::cout << "Source Path: " << SOURCE_PATH << "\n";

        Window window{1920, 1080, "Hello there"};
        renderer::Renderer renderer{window};

        ECS::AssetSystem assetSystem;

        engine::engine engine{renderer, assetSystem};

        editor::SceneEditor sceneEditor{window, renderer, assetSystem};
        sceneEditor.configureViewport(renderer.getRenderPass(0)->getAttachmentImageView(4), renderer.getRenderPass(0)->getAttachmentImageView(1), renderer.getDefaultSampler(), renderer.getRenderPass(0)->extent);
        editor::EngineState state = editor::EngineState::PAUSED;

        engine.setViewerObject(sceneEditor.viewportEntity);

        try{
            auto currentTime = std::chrono::high_resolution_clock::now();
            //bool firstLoopPerFrame = true; not sure how to implement this one
            //int frames = 0; // <=== useful for debugging (add to while condition)
            while(!renderer.window.shouldClose()){
                glfwPollEvents();

                //get passed time
                auto newTime = std::chrono::high_resolution_clock::now();
                float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime-currentTime).count();
                currentTime = newTime;
                
                sceneEditor.updateState(frameTime, state);
                if(state == editor::EngineState::RUNNING) engine.updateGameState(frameTime);

                if(auto commandBuffer = renderer.beginFrame()) {
                    int frameIndex = renderer.getFrameIndex();
                    engine.renderGameState(commandBuffer, frameIndex, assetSystem.GetComponent<ECS::Camera>(sceneEditor.viewportEntity));
                    renderer.beginSwapChainRenderPass(commandBuffer);
                    sceneEditor.renderState(commandBuffer);
                    renderer.endSwapChainRenderPass(commandBuffer);

                }
                renderer.endFrame(); 

                if(state == editor::EngineState::RESET) state = editor::EngineState::PAUSED;
            }
            vkDeviceWaitIdle(renderer.device.device());

        } catch(const std::exception &e) {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    #ifndef CONSOLE
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
            LPSTR lpCmdLine, int nCmdShow) {
            return main(__argc, __argv);
        }
    #endif // CONSOLE
#endif // EDITOR
