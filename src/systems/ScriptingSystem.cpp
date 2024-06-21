#include "ScriptingSystem.hpp"
#include <fstream>

namespace engine {
    ScriptingSystem::ScriptingSystem() {
        std::cout << "Initializing Mono runtime..." << std::endl;

        mono_set_assemblies_path("C:/Program Files/mono/lib");
        domain = mono_jit_init("engine");

        appDomain = mono_domain_create_appdomain("MyAppDomain", nullptr);
        mono_domain_set(appDomain, true);

        assembly = LoadAssembly("C:/Users/mizer/dev/Vulkan-game-engine/Scripts/EngineScripting/bin/Debug/EngineScripting.dll");
        if(assembly == nullptr) std::cout << "Failed to load assembly!" << std::endl;

    }

    ScriptingSystem::~ScriptingSystem() {
        mono_jit_cleanup(domain);
    }

    void ScriptingSystem::update(float deltaTime, ECS::AssetSystem& assetManager) {
        for(auto const& entity : entities) {
            //get components ==================================================
            ECS::Transform& transform = assetManager.GetComponent<ECS::Transform>(entity);
            ECS::Script& script = assetManager.GetComponent<ECS::Script>(entity);

            //set data ==================================================
            MonoClassField* scriptDeltaTime = mono_class_get_field_from_name(script.objectClass, "deltaTime");
            mono_field_set_value(script.scriptObject, scriptDeltaTime, &deltaTime);

            MonoObject* exception = nullptr;
            void* params[] = {
                &transform.translation.x,
                &transform.translation.y,
                &transform.translation.z
            };

            MonoClass* parentClass = mono_class_get_parent(script.objectClass);
            MonoMethod* sendTransformMethod = mono_class_get_method_from_name(parentClass, "loadTransform", 3);
            mono_runtime_invoke(sendTransformMethod, script.scriptObject, params, &exception);

            //do thing ==================================================
            MonoMethod* method = mono_class_get_method_from_name(script.objectClass, "update", 0);
            if(method == nullptr) std::cout << "Failed to get method!" << std::endl;

            mono_runtime_invoke(method, script.scriptObject, nullptr, &exception);
            if(exception != nullptr) std::cout << "Exception thrown!" << std::endl;

            //get data ==================================================
            MonoMethod* getTransformMethod = mono_class_get_method_from_name(parentClass, "returnTransform", 0);
            MonoObject* result = mono_runtime_invoke(getTransformMethod, script.scriptObject, nullptr, &exception);
            if(exception != nullptr) std::cout << "Exception thrown!" << std::endl;

            transform.translation = *(glm::vec3*)mono_object_unbox(result);

        }
    }

    //I like this error handling.
    MonoAssembly* ScriptingSystem::LoadAssembly(const std::string& assemblyPath) {
        //this first part can be abstacted as a function to load a file into a buffer
        std::ifstream stream(assemblyPath, std::ios::binary | std::ios::ate);
        if(!stream) return nullptr;
        
        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        int size = end - stream.tellg();
        if(size == 0) return nullptr;

        char* buffer = new char[size];
        stream.read((char*)buffer, size);
        stream.close();

        //==================================================================

        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(buffer, size, 1, &status, 0);
        delete[] buffer;
        if(status != MONO_IMAGE_OK) return nullptr;

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
        mono_image_close(image);

        return assembly;

    }

    void ScriptingSystem::printAssemblyMetadata(MonoAssembly* assembly) {
        
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++) {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            printf("%s.%s\n", nameSpace, name);
        }
    }
}
