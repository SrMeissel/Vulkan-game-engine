#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include "Components.hpp"

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
		entityManager = std::make_unique<EntityManager>();
		systemManager = std::make_unique<SystemManager>();

		RegisterComponent<Transform>();
		RegisterComponent<Camera>();
		RegisterComponent<Renderable>();
		RegisterComponent<Material>();
		RegisterComponent<Script>();
		RegisterComponent<PointLight>();
		RegisterComponent<SpotLight>();
		RegisterComponent<SkyBox>();
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
		entityManager->DestroyEntity(entity); // do the thing 
		componentManager->EntityDestroyed(entity); // deal with the repercussions
		systemManager->EntityDestroyed(entity); // good motto.
		savedComponents.erase(entity);
	}

    // Component functions ===========================================================

    template<typename T>
	void RegisterComponent() {
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		componentManager->RegisterComponent<T>();
	}

    template<typename T>
	T& AddComponent(Entity entity, T component) {
		T& placedComponent = componentManager->AddComponent<T>(entity, component); // do the thing
		savedComponents[entity].push_back(&placedComponent);

		auto signature = entityManager->GetSignature(entity); // make everyone aware that you did the thing
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

	template<typename T>
	ComponentType GetComponentType() {
		return componentManager->GetComponentType<T>();
	}

    // System functions ===========================================================

	template<typename T, typename... Args>
	std::shared_ptr<T> RegisterSystem(Args&&... args) {
		return systemManager->RegisterSystem<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	void SetSystemSignature(Signature signature) {
		systemManager->SetSignature<T>(signature);
	}

	template<typename T>
	void SetSystemAntiSignature(Signature signature) {
		systemManager->SetAntiSignature<T>(signature);
	}

	// SaveData functions ===========================================================

	std::unordered_map<Entity, std::vector<Component*>>& getAllEntities() {
		return savedComponents;
	}

    private:
        std::unique_ptr<ComponentManager> componentManager;
        std::unique_ptr<EntityManager> entityManager;
        std::unique_ptr<SystemManager> systemManager;

		//uses inheritence and virtual functions, is slow but will not be needed each frame, so its ok :)
		std::unordered_map<Entity, std::vector<Component*>> savedComponents;
    };
}