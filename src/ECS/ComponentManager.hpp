#pragma once

#include "EntityManager.hpp"

namespace ECS {
    class ComponentArrayParent {
    public:
	    virtual ~ComponentArrayParent() = default;
	    virtual void EntityDestroyed(Entity entity) = 0;
    };

    //used to manage data of a component type for all entities and ensures data packing
    template<typename T>
    class ComponentArray : public ComponentArrayParent {
    public:

    private:
    
    };

}