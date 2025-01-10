#include "engine.hpp"

#include "windowManager.hpp"
#include "Pipeline/Renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {



    Window window{1920, 1080, "Hello there"};
    renderer::Renderer renderer{window};

    ECS::AssetSystem assetSystem;

    SceneEditor sceneEditor{window, renderer};
    engine::engine engine{renderer, assetSystem};



    try{
        engine.run();
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}