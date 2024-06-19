#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <iostream>

#include "ECS/AssetManager.hpp"

#include "ECS/AssetManager.hpp"
#include "ECS/Components.hpp"

// https://www.mono-project.com/docs/advanced/embedding/
// if it breaks, copilot did it

// https://nilssondev.com/mono-guide/book/

namespace engine {
    class ScriptingSystem {
    public:
        ScriptingSystem();
        ~ScriptingSystem();

        MonoAssembly* LoadAssembly(const std::string& assemblyPath);
        void printAssemblyMetadata(MonoAssembly* assembly);

        void DoAThing() {
            std::cout << "Doing the second thing!" << std::endl;
        }

        void update(float deltaTime, int entity, ECS::AssetSystem& assetManager);

    private:
        MonoDomain* domain;
        MonoDomain* appDomain;
        MonoAssembly* assembly;
    };
}