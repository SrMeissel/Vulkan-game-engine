#pragma once

#include <unordered_map>

#include "EntityManager.hpp"

namespace ECS {
    //needed to communicate with generic component classes? unsure
    class ComponentArrayParent {
    public:
	    virtual ~ComponentArrayParent() = default;
	    virtual void EntityDestroyed(Entity entity) = 0;
    };

    //used to manage data of a component type for all entities and ensures data packing
    template<typename T>
    class ComponentArray : public ComponentArrayParent {
    public:
        void InsertData(Entity entity, T component) {
    		assert(entityToIndex.find(entity) == entityToIndex.end() && "Component added to same entity more than once.");

        size_t newIndex = arraySize;
		entityToIndex[entity] = newIndex;
		indexToEntity[newIndex] = entity;
		componentArray[newIndex] = component;
		++arraySize;
        }

        void RemoveData(Entity entity) {
            assert(entityToIndex.find(entity) != entityToIndex.end() && "Removing non-existent component.");

            //instead of deleting any information, the data at the end of the array is moved to the stale spot
            size_t indexOfRemovedEntity = entityToIndex[entity];
		    size_t indexOfLastElement = arraySize - 1;
		    componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

            // Update map to point to moved spot. I don't fully get this part.
		    Entity entityOfLastElement = indexToEntity[indexOfLastElement];
		    entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
		    indexToEntity[indexOfRemovedEntity] = entityOfLastElement;

            entityToIndex.erase(entity);
		    indexToEntity.erase(indexOfLastElement);

		    --arraySize;
        }

        T& GetData(Entity entity) {
		    assert(entityToIndex.find(entity) != entityToIndex.end() && "Data doesnt exist");

		    // Return a reference to the entity's component
	    	return componentArray[entityToIndex[entity]];
    	}

        //parent function override
        void EntityDestroyed(Entity entity) override {
		if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end()) {
			    // Remove the entity's component if it existed
			    RemoveData(entity);
		    }
	    }

    private:
        std::array<T, MAX_ENTITIES> componentArray; //THE data. kept packed
        size_t arraySize; //current count of entries in the array

        std::unordered_map<Entity, size_t> entityToIndex;
        std::unordered_map<size_t, Entity> indexToEntity;
    };

}