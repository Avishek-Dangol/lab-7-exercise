#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <array>
#include <cassert>
#include <cstddef>  // size_t
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "pack.h"  // Pack from prep
#include "registry.h"

using EntityID = size_t;

template <typename... ComponentTypes>
class EntityManager {
    using Components = Pack<ComponentTypes...>;
    static constexpr size_t NUM_COMPONENTS = sizeof...(ComponentTypes);

public:
    static constexpr size_t INVALID_COMPONENT_IDX = std::numeric_limits<size_t>::max();

    EntityID createEntity() {
        EntityID id;
        if (!removedIDs.empty()) {
            // Recycled IDs take priority
            id = removedIDs.back();
            removedIDs.pop_back();
            isAlive[id] = true;
        } else {
            // No recycled IDs, so make a new one
            id = isAlive.size();
            isAlive.push_back(true);
            componentIdx.emplace_back();
        }

        componentIdx[id].fill(INVALID_COMPONENT_IDX);
        return id;
    }

    bool validEntity(EntityID id) const { return id < isAlive.size() && isAlive[id]; }
    template <typename ComponentType>
    bool hasComponent(EntityID id) const {
        assert(validEntity(id) && "hasComponent on an invalid entity");
        // If the entity is valid, then the componentIdx entry must be valid too
        return getRegistry<ComponentType>().valid(componentIdx[id][typeIndex<ComponentType>()]);
    }

    template <typename ComponentType>
    ComponentType& getComponent(EntityID id) {
        assert(hasComponent<ComponentType>(id) && "entity does not have this component");
        // return entity's component by looking up the ticket in the registry for that component
        // type
        return getRegistry<ComponentType>()[componentIdx[id][typeIndex<ComponentType>()]];
    }

    template <typename ComponentType>
    const ComponentType& getComponent(EntityID id) const {
        assert(hasComponent<ComponentType>(id) && "entity does not have this component");
        return getRegistry<ComponentType>()[componentIdx[id][typeIndex<ComponentType>()]];
    }

    template <typename ComponentType>
    void addComponent(EntityID id, ComponentType&& component) {
        using C = std::remove_cvref_t<ComponentType>;
        assert(validEntity(id) && "addComponent on an invalid entity");
        assert(!hasComponent<C>(id) && "entity already has this component");
        // Store the component in the appropriate registry and record the ticket in componentIdx
        componentIdx[id][typeIndex<C>()] =
            getRegistry<C>().store(std::forward<ComponentType>(component));
    }

private:
    template <typename ComponentType>
    static constexpr size_t typeIndex() {
        static_assert(Components::template contains<ComponentType>,
                      "ComponentType is not managed by this EntityManager");
        return static_cast<size_t>(Components::template index<ComponentType>);
    }

    template <typename ComponentType>
    Registry<ComponentType>& getRegistry() {
        return std::get<typeIndex<ComponentType>()>(mRegistries);
    }

    template <typename ComponentType>
    const Registry<ComponentType>& getRegistry() const {
        return std::get<typeIndex<ComponentType>()>(mRegistries);
    }

    // tuple of registries for each component type (SoA)
    std::tuple<Registry<ComponentTypes>...> mRegistries;
    std::vector<std::array<size_t, NUM_COMPONENTS>> componentIdx;
    std::vector<bool> isAlive;
    std::vector<EntityID> removedIDs;
};

#endif  // ENTITY_MANAGER_H
