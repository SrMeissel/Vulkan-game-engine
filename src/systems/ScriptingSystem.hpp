#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "ECS/AssetManager.hpp"
#include "Pipeline/windowManager.hpp"
#include "keyMap.h"

// https://www.mono-project.com/docs/advanced/embedding/

// https://nilssondev.com/mono-guide/book/

namespace engine {
    class ScriptingSystem{
    public:
        ScriptingSystem(Window& window, ECS::AssetSystem& AssetManager);
        ~ScriptingSystem();

        MonoAssembly* LoadAssembly(const std::string& assemblyPath);
        void printAssemblyMetadata(MonoAssembly* assembly);

        void update(float deltaTime, ECS::AssetSystem& assetManager);

        static bool DoSomething(ScriptingSystem* system, int key) {
            if(system->window.isKeyDown(Key::KEY_LEFT_SHIFT) == true) { 
                return true;
            }
            return false;
        }

        struct Object {
            glm::vec3 position;
            glm::vec3 rotation;
        };

        Window& window;

        MonoDomain* domain;
        MonoDomain* appDomain;
        MonoAssembly* assembly;

        const std::string systemName{"Script"};
    private:
        std::shared_ptr<ECS::System> entities;
    };
}
