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

        Window window{1280, 720, "Hello there"};
        renderer::Renderer renderer{window};

        ECS::AssetSystem assetSystem;

        engine::engine engine{renderer, assetSystem};

	ECS::Entity viewportEntity = assetSystem.CreateEntity();
	assetSystem.AddComponent(viewportEntity, ECS::Transform{glm::vec3(0.0f, -3.5f, -12.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f)});
	assetSystem.AddComponent(viewportEntity, ECS::Camera{0.1, 5000});

        engine.setViewerObject(viewportEntity);

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

                    engine.renderGameState(assetSystem.GetComponent<ECS::Camera>(viewportEntity));

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
