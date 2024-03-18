#pragma once

#include <set>
#include <unordered_map>
#include <memory>
#include <typeinfo>

#include "EntityManager.hpp"

namespace ECS {
    
    //base class to be used by all systems
    // I'm not a great fan of having to inherit for each system but we'll see, this seems simple enough
    class System {
    public:
        std::set<Entity> entities;
    };

    //this is very similar to component array
    class SystemManager {
    public:
		//I just used copilot for the first time to modify this function, its awesome
        template<typename T, typename... Args>
        std::shared_ptr<T> RegisterSystem(Args&&... args) {
            const char* typeName = typeid(T).name();

            assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

            // Create a pointer to the system and return it so it can be used externally
            auto system = std::make_shared<T>(std::forward<Args>(args)...);
            //why is this different than everything else, VScode doesnt like it
            systems.insert({typeName, system});
            return system;
        }

    template<typename T>
	void SetSignature(Signature signature) {
		const char* typeName = typeid(T).name();

		assert(systems.find(typeName) != systems.end() && "System used before registered.");

		signatures.insert({typeName, signature});
	}
    
    void EntityDestroyed(Entity entity) {
		// entities is a set so no check needed. thats cool
		for (auto const& pair : systems)
		{
			auto const& system = pair.second;

			system->entities.erase(entity);
		}
	}

	void EntitySignatureChanged(Entity entity, Signature entitySignature) {
		// Notify each system that an entity's signature changed
		for (auto const& pair : systems)
		{
			auto const& type = pair.first;
			auto const& system = pair.second;
			auto const& systemSignature = signatures[type];

			// Entity signature matches system signature - insert into set
            //this is a cool bit of bitwise op's. the entity AND the signiture return the signiture if the entity contains all bits of the signiture :)
			if ((entitySignature & systemSignature) == systemSignature)
			{
				system->entities.insert(entity);
			}
			// Entity signature does not match system signature - erase from set
			else
			{
				system->entities.erase(entity);
			}
		}
	}


    private:        
        std::unordered_map<const char*, Signature> signatures{};
        std::unordered_map<const char*, std::shared_ptr<System>> systems{};
    };
}