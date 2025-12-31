#pragma once

#include <utility>

#include <entt/entt.hpp>

#include "detail.hpp"
#include "log.hpp"

namespace kine
{

class ECS;

class Entity
{
   public:
    Entity() = default;

    Entity(ECS* ecs_, entt::entity handle_) : ecs(ecs_), handle(handle_) {}

    bool valid() const;
    void destroy();

    ECS* get_ecs();

    template <typename T, typename... Args>
    T& add(Args&&... args);

    template <typename T>
    void remove();

    template <typename T>
    bool has() const;

    template <typename T>
    T& get();

    template <typename T, typename... Args>
    T& add_or_get(Args&&... args);

    entt::entity raw() const { return handle; }

    explicit operator bool() const { return valid(); }

   private:
    ECS* ecs = nullptr;
    entt::entity handle = entt::null;
};

template <typename View>
class EntityView
{
   public:
    class iterator
    {
       public:
        using underlying_iterator = typename View::iterator;

        iterator(ECS* ecs_, underlying_iterator it_) : ecs(ecs_), it(it_) {}

        iterator& operator++()
        {
            ++it;
            return *this;
        }

        bool operator!=(const iterator& other) const { return it != other.it; }

        Entity operator*() const { return Entity{ecs, *it}; }

       private:
        ECS* ecs;
        underlying_iterator it;
    };

    EntityView(ECS* ecs_, View view_) : ecs(ecs_), view(std::move(view_)) {}

    iterator begin() { return iterator{ecs, view.begin()}; }

    iterator end() { return iterator{ecs, view.end()}; }

    View& raw() { return view; }
    const View& raw() const { return view; }

   private:
    ECS* ecs;
    View view;
};

class ECS
{
   public:
    ECS() = default;

    ECS(const ECS&) = delete;
    ECS& operator=(const ECS&) = delete;

    Entity create() { return Entity{this, reg.create()}; }

    bool valid(entt::entity e) const { return reg.valid(e); }

    void destroy(entt::entity e)
    {
        if (reg.valid(e)) reg.destroy(e);
    }

    template <typename... Components>
    auto view()
    {
        auto v = reg.view<Components...>();
        return EntityView<decltype(v)>{this, v};
    }

    template <typename... Required, typename... Excluded>
    auto group()
    {
        auto g = reg.view<Required...>(entt::exclude<Excluded...>);
        return EntityView<decltype(g)>{this, g};
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

   private:
    friend class Entity;

    entt::registry reg;
};

// / ENTITY IMPL // /

inline bool Entity::valid() const { return ecs && ecs->valid(handle); }

inline void Entity::destroy()
{
    if (!ecs) return;

    ecs->destroy(handle);
    handle = entt::null;
    ecs = nullptr;
}

inline ECS* Entity::get_ecs() { return ecs; }

template <typename T, typename... Args>
T& Entity::add(Args&&... args)
{
    if (!valid()) LOG_THROW("Entity::add<%s>: invalid entity", TYPE_NAME(T));

    return ecs->reg.emplace<T>(handle, std::forward<Args>(args)...);
}

template <typename T>
void Entity::remove()
{
    if (!valid()) return;

    if (ecs->reg.all_of<T>(handle)) ecs->reg.remove<T>(handle);
}

template <typename T>
bool Entity::has() const
{
    return valid() && ecs->reg.all_of<T>(handle);
}

template <typename T>
T& Entity::get()
{
    if (!valid()) LOG_THROW("Entity::get<%s>: invalid entity", TYPE_NAME(T));

    if (!ecs->reg.all_of<T>(handle)) LOG_THROW("Entity::get<%s>: component does not exist", TYPE_NAME(T));

    return ecs->reg.get<T>(handle);
}

template <typename T, typename... Args>
T& Entity::add_or_get(Args&&... args)
{
    if (has<T>()) return get<T>();

    return add<T>(std::forward<Args>(args)...);
}

// / END ENTITY IMPL / //

template <typename... Components>
struct Requires
{
    static void attach(Entity e) { (e.add_or_get<Components>(), ...); }
};

}  // namespace kine
