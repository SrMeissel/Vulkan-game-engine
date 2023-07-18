#pragma once

#include "windowManager.hpp"

namespace engine {
    class app {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();
        private:
            Window window{WIDTH, HEIGHT, "Hello there"};
    };
}