#pragma once

#include <vector>
#include <array>
#include <queue>
#include <bitset>
#include <cassert>

//this will be my ECS system
// https://medium.com/@savas/nomad-game-engine-part-2-ecs-9132829188e5 
//this is great because it has nice pictures 
//https://austinmorlan.com/posts/entity_component_system/
//components just hold data. Systems hold logic.
//std::bitset
//This is cool ^, real cool 

//im going to replace the current system with one closely based off of austins exaple. Because its nice and I can understand it :)
//austin suggests using 2 maps, one entity/key pair, and a key/entity pair. why? no clue

//austin uses a type template to register types as components

//using a std::set to store keys for each system to iterate through

//my asset handler class will be my version of the coordinator
//but will include loading and saving things to disk (Ex. gltf stuff, editor things)

namespace engine {

using Entity = std::uint32_t;
static const Entity MAX_ENTITIES = 5000; //#def const? what diff? If I use a vector I dont need this constant

using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>; // used to determine which components an entity has

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

        void destroyEntity(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity can never have existed");

            signatures[entity].reset();

            //keeps data density by pushing!!!
            availableEntities.push(entity);
            --livingEntities;
        }

    private:
        std::array<Signature, MAX_ENTITIES> signatures{}; //could be vector. // each bit represents a component a entity has. // Will need to be changed if I want to have multiple of the same type of component, which I likely will.
        std::queue<Entity> availableEntities{}; 

        uint32_t livingEntities{}; //notice same type as Entity
    };
}