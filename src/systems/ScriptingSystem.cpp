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

        //printAssemblyMetadata(assembly);

        //call a function from the assembly

        // MonoImage* image = mono_assembly_get_image(assembly);
        // MonoClass* scriptClass = mono_class_from_name(image, "", "CSharpTesting");
        // if(scriptClass == nullptr) std::cout << "Failed to get class!" << std::endl;

        // MonoObject* scriptObject = mono_object_new(appDomain, scriptClass);
        // if(scriptObject == nullptr) std::cout << "Failed to create object!" << std::endl;

        // mono_runtime_object_init(scriptObject); // constructor

        // MonoClass* instanceClass = mono_object_get_class(scriptObject);

        // MonoMethod* method = mono_class_get_method_from_name(instanceClass, "PrintFloatVar", 0);
        // if(method == nullptr) std::cout << "Failed to get method!" << std::endl;

        // MonoClassField* floatField = mono_class_get_field_from_name(instanceClass, "MyPublicFloatVar");

        // // Get the value of MyPublicFloatVar from the testingInstance object
        // float value;
        // mono_field_get_value(scriptObject, floatField, &value);

        // //std::cout << "Value of MyPublicFloatVar: " << value << std::endl;

        // // Increment value by 10 and assign it back to the variable
        // value += 10.0f;
        // mono_field_set_value(scriptObject, floatField, &value);

        // MonoObject* exception = nullptr;
        // mono_runtime_invoke(method, scriptObject, nullptr, &exception);
    }

    ScriptingSystem::~ScriptingSystem() {
        mono_jit_cleanup(domain);
    }

    void ScriptingSystem::update(float deltaTime, int entity, ECS::AssetSystem& assetManager) {
        MonoImage* image = mono_assembly_get_image(assembly);
        MonoClass* scriptClass = mono_class_from_name(image, "", "TransformExpirement");
        if(scriptClass == nullptr) std::cout << "Failed to get class!" << std::endl;

        MonoObject* scriptObject = mono_object_new(appDomain, scriptClass);
        if(scriptObject == nullptr) std::cout << "Failed to create object!" << std::endl;

        mono_runtime_object_init(scriptObject); // constructor

        MonoClass* scriptInstance = mono_object_get_class(scriptObject);

        float& position = assetManager.GetComponent<ECS::Transform>(entity).translation.x;

        //set data
        MonoClassField* scriptDeltaTime = mono_class_get_field_from_name(scriptInstance, "deltaTime");
        mono_field_set_value(scriptObject, scriptDeltaTime, &deltaTime);
        MonoClassField* scriptPosition = mono_class_get_field_from_name(scriptInstance, "transformPositionX");
        mono_field_set_value(scriptObject, scriptPosition, &position);

        //do thing
        MonoMethod* method = mono_class_get_method_from_name(scriptInstance, "update", 0);
        if(method == nullptr) std::cout << "Failed to get method!" << std::endl;

        MonoObject* exception = nullptr;
        mono_runtime_invoke(method, scriptObject, nullptr, &exception);

        //get data
        mono_field_get_value(scriptObject, scriptPosition, &position);

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
