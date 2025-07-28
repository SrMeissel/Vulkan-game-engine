#pragma once

#include <unordered_map>

#include "EntityManager.hpp"

namespace ECS {
    class ComponentArrayParent {
    public:
	    virtual ~ComponentArrayParent() = default;
	    virtual void EntityDestroyed(Entity entity) = 0;
    };

    template<typename T>
    class ComponentArray : public ComponentArrayParent {
    using Deleter = std::function<void(T&)>;
    
    public:
        T& InsertData(Entity entity, T component) {
    		assert(entityToIndex.find(entity) == entityToIndex.end() && "Component added to same entity more than once.");

        size_t newIndex = arraySize;
		entityToIndex[entity] = newIndex;
		indexToEntity[newIndex] = entity;
		componentArray[newIndex] = component;
		++arraySize;

        return componentArray[newIndex];
        }

        void RemoveData(Entity entity) {
            assert(entityToIndex.find(entity) != entityToIndex.end() && "Removing non-existent component.");

            deleter(componentArray[entityToIndex[entity]]);

            //instead of deleting any information, the data at the end of the array is moved to the stale spot
            size_t indexOfRemovedEntity = entityToIndex[entity];
		    size_t indexOfLastElement = arraySize - 1;
		    componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

		    Entity entityOfLastElement = indexToEntity[indexOfLastElement];
		    entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
		    indexToEntity[indexOfRemovedEntity] = entityOfLastElement;

            entityToIndex.erase(entity);
		    indexToEntity.erase(indexOfLastElement);

		    --arraySize;
        }

        T& GetData(Entity entity) {
		    assert(entityToIndex.find(entity) != entityToIndex.end() && "Data doesnt exist");
	    	return componentArray[entityToIndex[entity]];
    	}

        void EntityDestroyed(Entity entity) override {
		if (entityToIndex.find(entity) != entityToIndex.end()) {
			    RemoveData(entity);
		    }
	    }

    private:
        std::array<T, MAX_ENTITIES> componentArray;
        size_t arraySize;

        std::unordered_map<Entity, size_t> entityToIndex;
        std::unordered_map<size_t, Entity> indexToEntity;

        Deleter deleter;
    };

}