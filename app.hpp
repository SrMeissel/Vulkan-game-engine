#pragma once

#include "windowManager.hpp"
#include "pipeline.hpp"
#include "deviceManager.hpp"

namespace engine {
    class app {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();
        private:
            Window window{WIDTH, HEIGHT, "Hello there"};
            Device Device{window};
            Pipeline Pipeline{Device, "../../shaders/simple.vert.spv" , "../../shaders/simple.frag.spv", Pipeline::defaultPipelineConifInfo(WIDTH, HEIGHT)};
    };
}