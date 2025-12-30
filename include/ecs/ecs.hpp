#pragma once

#include <cstdint>
#include <entt/entt.hpp>

#include "detail.hpp"
#include "log.h"

namespace kine
{

using Entity = entt::entity;

class ECS
{
   public:
    ECS() = default;

    ECS(const ECS&) = delete;
    ECS& operator=(const ECS&) = delete;

    Entity create() { return reg.create(); }

    void destroy_entity(Entity entity)
    {
        if (reg.valid(entity)) reg.destroy(entity);
    }

    bool is_valid(Entity entity) const { return reg.valid(entity); }

    template <typename T, typename... Args>
    T& add_component(Entity entity, Args&&... args)
    {
        if (!reg.valid(entity))
            LOG_THROW("ECS::add_component<%s>: invalid entity %d", TYPE_NAME(T), static_cast<std::uint32_t>(entity));

        return reg.emplace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void remove_component(Entity entity)
    {
        if (!reg.valid(entity)) return;

        if (reg.all_of<T>(entity)) reg.remove<T>(entity);
    }

    template <typename T>
    bool has_component(Entity entity) const
    {
        return reg.valid(entity) && reg.all_of<T>(entity);
    }

    template <typename T>
    T& get_component(Entity entity)
    {
        if (!reg.valid(entity))
            LOG_THROW("ECS::add_component<%s>: invalid entity %d", TYPE_NAME(T), static_cast<std::uint32_t>(entity));

        if (!reg.all_of<T>(entity))
            LOG_THROW("ECS::add_component<%s>: invalid entity %d", TYPE_NAME(T), static_cast<std::uint32_t>(entity));

        return reg.get<T>(entity);
    }

    template <typename T, typename... Args>
    T& add_or_get_component(Entity entity, Args&&... args)
    {
        if (reg.all_of<T>(entity)) return get_component<T>(entity);

        return add_component<T>(entity, std::forward<Args>(args)...);
    }

    template <typename... Components>
    auto view()
    {
        return reg.view<Components...>();
    }

    template <typename... Required, typename... Excluded>
    auto group()
    {
        // TODO: still using view until group config is finalized
        return reg.view<Required...>(entt::exclude<Excluded...>);
    }

    template <typename T, typename... Args>
    T& set_context(Args&&... args)
    {
        return reg.ctx().emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    T& get_context()
    {
        if (!reg.ctx().contains<T>()) LOG_THROW("ECS::get_context<%s>: context does not exist", TYPE_NAME(T));

        return reg.ctx().get<T>();
    }

    template <typename T>
    bool has_context() const
    {
        return reg.ctx().contains<T>();
    }

    template <typename T>
    void remove_context()
    {
        reg.ctx().erase<T>();
    }

    entt::registry& registry() { return reg; }
    const entt::registry& registry() const { return reg; }

   private:
    entt::registry reg;
};

template <typename... Components>
struct Requires
{
    static void attach(ECS& ecs, Entity e) { (ecs.add_or_get_component<Components>(e), ...); }
};

}  // namespace kine
