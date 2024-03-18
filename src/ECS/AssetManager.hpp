#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

namespace ECS {
    class AssetSystem {
    public:
    //not just constructor? seems like a constructor to me.
	void Init() {
		componentManager = std::make_unique<ComponentManager>();
		entityManager = std::make_unique<EntityManager>();
		systemManager = std::make_unique<SystemManager>();
	}

    // Entity functions ===========================================================

    Entity CreateEntity() {
		return entityManager->CreateEntity();
	}

	void DestroyEntity(Entity entity) {
		entityManager->DestroyEntity(entity); // do the thing 
		componentManager->EntityDestroyed(entity); // deal with the repercussions
		systemManager->EntityDestroyed(entity); // good motto.
	}

    // Component functions ===========================================================

    template<typename T>
	void RegisterComponent() {
		componentManager->RegisterComponent<T>();
	}

    template<typename T>
	void AddComponent(Entity entity, T component) {
		componentManager->AddComponent<T>(entity, component); // do the thing

		auto signature = entityManager->GetSignature(entity); // make everyone aware that you did the thing
		signature.set(componentManager->GetComponentType<T>(), true);
		entityManager->SetSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}
    
	template<typename T>
	void RemoveComponent(Entity entity) {
		componentManager->RemoveComponent<T>(entity);

		auto signature = entityManager->GetSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), false);
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


    private:
        std::unique_ptr<ComponentManager> componentManager;
        std::unique_ptr<EntityManager> entityManager;
        std::unique_ptr<SystemManager> systemManager;
    };
}