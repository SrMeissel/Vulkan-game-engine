#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include "Components.hpp"
#include "ComplementManager.hpp"

#include <type_traits>
#include <random>
#include <chrono>

namespace ECS {
    inline uint64_t generateEntityID() {
        uint64_t time = std::chrono::steady_clock::now().time_since_epoch().count();
        static std::mt19937 rng(std::random_device{}());
        uint32_t id = rng();
        return (time << 32) | id;
    }

    class AssetSystem {
    public:

	AssetSystem() {
		componentManager = std::make_unique<ComponentManager>();
		complementManager = std::make_unique<ComplementManager>();
		entityManager = std::make_unique<EntityManager>();
		systemManager = std::make_unique<SystemManager>();
	}

    // Entity functions ===========================================================

    Entity CreateEntity(Entity entity = 0) {
		if(entity == 0) entity = generateEntityID();
		entityManager->CreateEntity();
		std::vector<Component*> components;
        savedComponents.insert({entity, components});		
		return entity;
	}

	void DestroyEntity(Entity entity) {
		entityManager->DestroyEntity(entity);
		componentManager->EntityDestroyed(entity);
		systemManager->EntityDestroyed(entity);
		savedComponents.erase(entity);
	}

    // Component functions ===========================================================

    template<typename T>
	void RegisterComponent(std::function<void(const T&)> cleaner = [](const T&){}) {
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		componentManager->RegisterComponent<T>(cleaner);
	}

	template<typename Cmpl>
	void RegisterComplement(std::function<void*(Entity entity)> toComplement) {
		complementManager->registerComplement<Cmpl>(toComplement);
	}

    template<typename T>
	T& AddComponent(Entity entity, T component) {
		T& placedComponent = componentManager->AddComponent<T>(entity, component);
		savedComponents[entity].push_back(&placedComponent);

		auto signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), true);
		entityManager->SetSignature(entity, signature);
		systemManager->EntitySignatureChanged(entity, signature);

		return placedComponent;
	}
    
	template<typename T>
	void RemoveComponent(Entity entity) {
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		savedComponents[entity].erase(std::remove(savedComponents[entity].begin(), savedComponents[entity].end(), &(componentManager->GetComponent<T>(entity))), savedComponents[entity].end());
		//hell yeah brother

		componentManager->RemoveComponent<T>(entity);

		auto signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), false);
		entityManager->SetSignature(entity, signature);

		systemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	T& GetComponent(Entity entity) {
		return componentManager->GetComponent<T>(entity);
	}

	template<typename Cmpl>
	void* GetComplement(Entity entity) {
		return complementManager->getComplement<Cmpl>(entity);
	};

	void* GetComplement(Entity entity, ComponentType type) {
		return complementManager->getComplement(entity, type);
	}

	template<typename T>
	ComponentType GetComponentType() {
		return componentManager->GetComponentType<T>();
	}

    // System functions ===========================================================

	std::shared_ptr<System> RegisterSystem(std::string name) {
		return systemManager->RegisterSystem(name);
	}

	void SetSystemSignature(Signature signature, std::string name) {
		systemManager->SetSignature(signature, name);
	}

	void SetSystemAntiSignature(Signature signature, std::string name) {
		systemManager->SetAntiSignature(signature, name);
	}

	// SaveData functions ===========================================================

	std::vector<Entity> getAllEntities() {

		std::vector<Entity> Entities;
		Entities.reserve(savedComponents.size());
		for(auto pair : savedComponents) {
			Entities.push_back(pair.first);
		}
		return Entities;
	}

    private:
        std::unique_ptr<ComponentManager> componentManager;
        std::unique_ptr<EntityManager> entityManager;
        std::unique_ptr<SystemManager> systemManager;
		std::unique_ptr<ComplementManager> complementManager;

		std::unordered_map<Entity, std::vector<Component*>> savedComponents;
    };
}