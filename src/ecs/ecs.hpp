#pragma once

#include <entt/entt.hpp>
#include <log.hpp>
#include <stdexcept>

namespace kine
{
using Entity = entt::entity;

// Core ECS class encapsulating the entt::registry.
class ECS
{
   public:
    ECS() = default;

    ECS(const ECS&) = delete;
    ECS& operator=(const ECS&) = delete;

    Entity create() { return reg.create(); }

    void destroy_entity(Entity entity)
    {
        if (reg.valid(entity))
        {
            reg.destroy(entity);
        }
    }

    // Checks if an entity ID is valid (hasn't been destroyed).
    bool is_valid(Entity entity) const { return reg.valid(entity); }

    // Adds a component of type T to an entity.
    template <typename T, typename... Args>
    T& add_component(Entity entity, Args&&... args)
    {
        if (!reg.valid(entity)) LOG_THROW(std::runtime_error, "Attempted to add component to invalid entity.");
        return reg.emplace<T>(entity, std::forward<Args>(args)...);
    }

    // Removes a component of type T from an entity.
    template <typename T>
    void remove_component(Entity entity)
    {
        if (reg.valid(entity) && reg.all_of<T>(entity)) reg.remove<T>(entity);
        // Note: We avoid throwing if the component doesnt exist, as its often fine.
    }

    // Checks if an entity has a specific component.
    template <typename T>
    bool has_component(Entity entity) const
    {
        return reg.all_of<T>(entity);
    }

    // Gets a reference to a component of type T from an entity.
    template <typename T>
    T& get_component(Entity entity)
    {
        if (!reg.valid(entity) || !reg.all_of<T>(entity))
            LOG_THROW(std::runtime_error, "Attempted to get non-existent component or invalid entity.");
        return reg.get<T>(entity);
    }

    // Gets a view for iterating over entities that have all specified component types.
    template <typename... Components>
    auto view()
    {
        return reg.view<Components...>();
    }

    // Gets a group for fast iteration over entities that have all required components
    template <typename... RequiredComponents, typename... ExcludedComponents>
    auto group()
    {
        // TODO: Setup reg.group
        return reg.view<RequiredComponents...>(entt::exclude<ExcludedComponents...>);
    }

    // Creates or replaces a global context object.
    template <typename T, typename... Args>
    T& set_context(Args&&... args)
    {
        return reg.ctx().emplace<T>(std::forward<Args>(args)...);
    }

    // Gets a reference to the global context object.
    template <typename T>
    T& get_context()
    {
        if (!reg.ctx().contains<T>()) LOG_THROW(std::runtime_error, "Attempted to get non-existent context object.");
        return reg.ctx().get<T>();
    }

    // Checks if a global context object exists.
    template <typename T>
    bool has_context() const
    {
        return reg.ctx().contains<T>();
    }

    // Removes the global context object.
    template <typename T>
    void remove_context()
    {
        reg.ctx().erase<T>();
    }

    entt::registry& registry() { return reg; }

   private:
    entt::registry reg;
};
}  // namespace kine
