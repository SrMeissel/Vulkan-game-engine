#pragma once

#include <unordered_map>
#include <memory>

#include "EntityManager.hpp"
#include "ComponentArray.hpp"

//I like that this uses type names as keys, its cool
//remember: any time something is given as a pointer, it is modifiable

namespace ECS {
    
    template<typename T>
    class ComponentManager {
    public:
        //this is a better solution than the current static GameObject ID function
        void RegisterComponent() { 
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) == componentTypes.end() && "Registering component type more than once.");

            //creates and stores the component ID and it's component array
            componentTypes.insert({typeName, nextComponentType});
            componentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});

            // Increment the value so that the next component registered will be different
            ++nextComponentType;
        }

        template<typename T>
	    ComponentType GetComponentType() {
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");

            // Return this component's type - used for creating signatures
            return componentTypes[typeName];
	    }

        template<typename T>
        void AddComponent(Entity entity, T component) {
            // Add a component to the array for an entity
            GetComponentArray<T>()->InsertData(entity, component);
        }

        template<typename T>
        void RemoveComponent(Entity entity) {
            // Remove a component from the array for an entity
            GetComponentArray<T>()->RemoveData(entity);
        }


    private:
        //takes in type ID's as keys
        std::unordered_map<const char*, ComponentType> componentTypes{};
        std::unordered_map<const char*, std::shared_ptr<ComponentArray>> componentArrays{};

        ComponentType nextComponentType{};

        //this is private for a reason, im sure I'll figure out why
        //I think by its use in AddComponent and RemoveComponent, It keeps the array private and harder to misuse
        //guess thats why its in "private".
        template<typename T>
        std::shared_ptr<ComponentArray<T>> GetComponentArray() {
            const char* typeName = typeid(T).name();

            assert(componentTypes.find(typeName) != componentTypes.end() && "Component needs to be registered first. ");

            return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
        }

    };
}