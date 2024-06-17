#include "ScriptingSystem.hpp"

namespace engine {
    ScriptingSystem::ScriptingSystem() {
        std::cout << "Initializing Mono runtime..." << std::endl;

        mono_set_assemblies_path("C:/Program Files/mono/lib");

        try{
            domain = mono_jit_init("engine");
            if(domain == nullptr) {
                throw std::runtime_error("Failed to initialize Mono domain");
            }
        } catch (std::exception& e) {
            std::cerr << "Failed to initialize Mono runtime: " << e.what() << std::endl;
        }

    }

    ScriptingSystem::~ScriptingSystem() {
        mono_jit_cleanup(domain);
    }

}
