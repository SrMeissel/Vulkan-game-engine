#pragma once

#include <set>
#include <unordered_map>
#include <memory>

#include "EntityManager.hpp"

namespace ECS {
    	using System = std::set<Entity>;

    //this is very similar to component array
    class SystemManager {
    public:

        std::shared_ptr<System> RegisterSystem(std::string name) {
            assert(systems.find(name) == systems.end() && "Registering system more than once.");

            auto system = std::make_shared<System>();
            systems.insert({name, system});
            return system;
        }

	void SetSignature(Signature signature, std::string name) {
		assert(systems.find(name) != systems.end() && "System used before registered.");

		signatures.insert({name, signature});
	}

	void SetAntiSignature(Signature signature, std::string name) {
		assert(systems.find(name) != systems.end() && "System used before registered.");

		antiSignatures.insert({name, signature});
	}
    
    void EntityDestroyed(Entity entity) {
		for (auto const& pair : systems)
		{
			auto const& system = pair.second;

			system->erase(entity);
		}
	}

	void EntitySignatureChanged(Entity entity, Signature entitySignature) {
		// Notify each system that an entity's signature changed
		for (auto const& pair : systems)
		{
			auto const& type = pair.first;
			auto const& system = pair.second;
			auto const& systemSignature = signatures[type];
			auto const& antiSignature = antiSignatures[type];

			// Entity signature matches system signature - insert into set
            //this is a cool bit of bitwise op's. the entity AND the signiture return the signiture if the entity contains all bits of the signiture :)
			if (((entitySignature & systemSignature) == systemSignature) && ((entitySignature & antiSignature) == 0))
			{
				system->insert(entity);
			}
			// Entity signature does not match system signature - erase from set
			else
			{
				system->erase(entity);
			}
		}
	}

    private:        
        std::unordered_map<std::string, Signature> signatures{};
		std::unordered_map<std::string, Signature> antiSignatures{};
        std::unordered_map<std::string, std::shared_ptr<System>> systems{};
    };
}