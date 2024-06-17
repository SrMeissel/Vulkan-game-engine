#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <iostream>

#include "ECS/AssetManager.hpp"

// https://www.mono-project.com/docs/advanced/embedding/
// if it breaks, copilot did it

// https://nilssondev.com/mono-guide/book/

namespace engine {
    class ScriptingSystem {
    public:
        ScriptingSystem();
        ~ScriptingSystem();

        void LoadAssembly(const std::string& assemblyPath);
        void RunScript(const std::string& scriptName);

        void DoAThing() {
            std::cout << "Doing the second thing!" << std::endl;
        }

    private:
        MonoDomain* domain;
        MonoAssembly* assembly;
    };
}