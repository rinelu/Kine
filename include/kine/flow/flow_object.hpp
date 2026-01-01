#pragma once
#include <memory>
#include <string>
#include <vector>

#include "kine/ecs/ecs.hpp"

namespace kine
{

class FlowObject
{
   public:
    virtual ~FlowObject() = default;

    std::string name;
    FlowObject* parent = nullptr;
    std::vector<std::unique_ptr<FlowObject>> children;

    Entity entity;

    virtual void on_attach() {}
    virtual void on_detach() {}
    virtual void init() {}
    virtual void update(float) {}
    virtual void fixed_update(float) {}

    template <typename T, typename... Args>
    T* add_child(const std::string& child_name, Args&&... args)
    {
        static_assert(std::is_base_of_v<FlowObject, T>, "T must derive from FlowObject");

        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->name = child_name;
        child->parent = this;
        child->entity = entity.get_ecs()->create();

        T* raw = child.get();
        children.push_back(std::move(child));
        return raw;
    }

    template <typename T>
    T* find()
    {
        static_assert(std::is_base_of_v<FlowObject, T>, "T must derive from FlowObject");

        if (auto* self = dynamic_cast<T*>(this)) return self;

        for (auto& child : children)
            if (auto* found = child->find<T>()) return found;

        return nullptr;
    }

    template <typename T>
    void find_all(std::vector<T*>& out)
    {
        static_assert(std::is_base_of_v<FlowObject, T>, "T must derive from FlowObject");
        if (auto* self = dynamic_cast<T*>(this)) out.push_back(self);
        for (auto& child : children) child->find_all<T>(out);
    }

    FlowObject* find(const std::string& targetName);
};

}  // namespace kine
