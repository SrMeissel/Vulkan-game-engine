#include "ScriptingSystem.hpp"
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
#define MONO_ASSEMBLIES "C:/Program Files/mono/lib/mono/4.5"
#else
#define MONO_ASSEMBLIES "/usr/lib/mono/4.5" 
#endif
// https://mono.github.io/mail-archives/mono-list/2015-November/051922.html
// WHY TF IS THIS THE BEST DOCS I CAN FIND

//replacement if needed
//https://github.com/dotnet/runtime/discussions/79309

namespace engine {
    ScriptingSystem::ScriptingSystem(Window& window) : window{window} {
        std::cout << "Initializing Mono runtime..." << std::endl;
    
        //_putenv_s("MONO_GC_PARAMS", "nursery-size=64m");

        mono_set_assemblies_path(MONO_ASSEMBLIES);
        domain = mono_jit_init("engine");

        appDomain = mono_domain_create_appdomain("MyAppDomain", nullptr);
        mono_domain_set(appDomain, true);

        assembly = LoadAssembly((std::string)SOURCE_PATH + "/Scripts/EngineScripting/bin/Debug/EngineScripting.dll");
        if(assembly == nullptr) throw std::runtime_error("Bad Script Assembly");

        mono_add_internal_call("GameEngine.EngineCore::DoSomething", reinterpret_cast<const void*>(&DoSomething));

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

            MonoClassField* scriptThingy = mono_class_get_field_from_name(script.objectClass, "thingy");
            mono_field_set_value(script.scriptObject, scriptThingy, (char*)this); 

            //unmanaged thunks
            MonoException* loadTransformException = NULL;
            script.loadTransform(script.scriptObject, transform.translation.x, transform.translation.y, transform.translation.z, transform.rotation.x, transform.rotation.y, transform.rotation.z, &loadTransformException);

            //do thing ==================================================

            MonoException* updateException = NULL;
            script.update(script.scriptObject, &updateException);
            if(updateException != nullptr) std::cout << "Exception thrown!" << std::endl;

            //get data ==================================================

            MonoClassField* objectField = mono_class_get_field_from_name(script.objectClass, "Object");
            Object object;
            mono_field_get_value(script.scriptObject, objectField, &object);
            transform.translation = object.position;
            transform.rotation = object.rotation;
        }
    }



    //I like this error handling.
    MonoAssembly* ScriptingSystem::LoadAssembly(const std::string& assemblyPath) {
        //this first part can be abstacted as a function to load a file into a buffer
        std::ifstream stream(assemblyPath, std::ios::binary | std::ios::ate);
        if(!stream) return nullptr;
        
        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        int size = (int)(end - stream.tellg());
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

    //this will become important.
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
