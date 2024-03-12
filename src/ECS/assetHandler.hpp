#pragma once

#include <vector>

#include "gameObject.hpp"

//this will be my ECS system
// https://medium.com/@savas/nomad-game-engine-part-2-ecs-9132829188e5 
//this is great because it has nice pictures 
//https://austinmorlan.com/posts/entity_component_system/
//components just hold data. Systems hold logic.
//std::bitset
//This is cool ^, real cool 

//im going to replace the current system with one closely based off of austins exaple. Because its nice and I can understand it :)
//austin suggests using 2 maps, one entity/key pair, and a key/entity pair. why? no clue

//austin uses a type template to register types as component types

namespace engine {
    class assetHandler {
    public:
        assetHandler();

        GameObject getObject(GameObject::id_t key);

        //new object
        GameObject::id_t createObject();
        //delete object
        void deleteObject(GameObject::id_t key);
        //modify object
            //add component
                void setModel(GameObject::id_t key);
                void setTexture(GameObject::id_t key);
                void setPhysicsObject(GameObject::id_t key);
                //...
            //modify component
            //remove component

        //save map to disk
        //load map from disk
    private:
        //unordered map of gameobjects
        
        // lists of keys for objects that have specific components
        std::vector<GameObject::id_t> modelKeys;
        std::vector<GameObject::id_t> textureKeys;
        std::vector<GameObject::id_t> physicsKeys;
    };
}