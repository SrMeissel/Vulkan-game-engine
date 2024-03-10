#pragma once

#include <vector>

#include "gameObject.hpp"

namespace engine {
    class assetHandler {
    public:
        assetHandler();

        void getObject(GameObject::id_t key);

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