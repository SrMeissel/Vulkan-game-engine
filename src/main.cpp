#include "engine.hpp"

#include "Pipeline/windowManager.hpp"
#include "Pipeline/Renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <chrono>

int main(int argc, char** argv) {

    Window window{1920, 1080, "Hello there"};
    std::cout << "window created \n";
    renderer::Renderer renderer{window};
    std::cout << "Backend Systems created \n";

    ECS::AssetSystem assetSystem;

    engine::engine engine{renderer, assetSystem};
    engine.init();
    std::cout << "engine created \n";

    editor::SceneEditor sceneEditor{window, renderer};
    std::cout << "editor started \n";
    sceneEditor.configureViewport(renderer.getRenderPass(0)->getAttachmentImageView(4), renderer.getRenderPass(0)->getAttachmentImageView(1), renderer.getDefaultSampler(), renderer.getRenderPass(0)->extent);
    std::cout << "editor created \n";

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

            engine.updateGameState(frameTime);

            if(auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                engine.renderGameState(commandBuffer, frameIndex);
                renderer.beginSwapChainRenderPass(commandBuffer);
                sceneEditor.runOnce(commandBuffer);
                renderer.endSwapChainRenderPass(commandBuffer);

            }

            renderer.endFrame(); 
        }
        vkDeviceWaitIdle(renderer.device.device());

        engine.cleanUp();
        engine.~engine();

    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}