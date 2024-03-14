#pragma once

#include <vector>
#include <array>
#include <queue>
#include <bitset>
#include <cassert>

namespace ECS {

using Entity = std::uint32_t;
static const Entity MAX_ENTITIES = 5000; //#def const? what diff? If I use a vector I dont need this constant

using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>; // used to determine which components an entity has
    //Entities are the ID's that accociate components together
    
    class EntityManager {
    public:
        EntityManager() {
            // The queue with all possible entities must be initialized. I think I can also use a static iteration function like the GameObject constructor but same thing
            for(Entity entity = 0; entity < MAX_ENTITIES; ++entity) { //why not entity++? 
                availableEntities.push(entity);
            }
        };

        Entity createEntity() {
            assert(livingEntities < MAX_ENTITIES && "TOO MANY THINGS!!");
            
            Entity id = availableEntities.front();
            availableEntities.pop();
            livingEntities++;

            return id;
        };

        //cleaning up stale component data of a deleted entity is handled elsewhere
        void destroyEntity(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity can never have existed");

            signatures[entity].reset();

            //keeps data density by pushing!!!
            availableEntities.push(entity);
            --livingEntities;
        }

        void SetSignature(Entity entity, Signature signature) {
            assert(entity < MAX_ENTITIES && "Entity can never have existed");

            signatures[entity] = signature;
        }
        Signature GetSignature(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity can never have existed");

            return signatures[entity];
        }

    private:
        std::array<Signature, MAX_ENTITIES> signatures{}; //could be vector. // each bit represents a component a entity has. // Will need to be changed if I want to have multiple of the same type of component, which I likely will.
        std::queue<Entity> availableEntities{}; 

        uint32_t livingEntities{}; //notice same type as Entity
    };
}