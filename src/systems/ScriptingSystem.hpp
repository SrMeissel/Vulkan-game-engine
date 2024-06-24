#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <iostream>

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"

#include "Pipeline/windowManager.hpp"

// https://www.mono-project.com/docs/advanced/embedding/
// if it breaks, copilot did it

// https://nilssondev.com/mono-guide/book/

namespace engine {
    class ScriptingSystem : public ECS::System{
    public:
        ScriptingSystem(Window& window);
        ~ScriptingSystem();

        MonoAssembly* LoadAssembly(const std::string& assemblyPath);
        void printAssemblyMetadata(MonoAssembly* assembly);

        void update(float deltaTime, ECS::AssetSystem& assetManager);

        static bool DoSomething(ScriptingSystem* system, int key) {
            if(glfwGetKey(system->window.getGLFWwindow(), key) == GLFW_PRESS) {
                return true;
            }
            return false;
        }

        Window& window;

        MonoDomain* domain;
        MonoDomain* appDomain;
        MonoAssembly* assembly;
    };
}