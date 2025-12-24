#pragma once
#include <ecs/ecs.hpp>
#include <memory>
#include <string>
#include <vector>

namespace kine
{

class FlowObject
{
   public:
    virtual ~FlowObject() = default;

    std::string name;
    FlowObject* parent = nullptr;
    std::vector<std::unique_ptr<FlowObject>> children;

    Entity entity{entt::null};

    virtual void on_attach(ECS&) {}
    virtual void on_detach(ECS&) {}
    virtual void init(ECS&) {}
    virtual void update(ECS&, float) {}
    virtual void tick(ECS&, float) {}

    template <typename T, typename... Args>
    T* add_child(const std::string& childName, Args&&... args)
    {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->name = childName;
        child->parent = this;

        T* raw = child.get();
        children.push_back(std::move(child));
        return raw;
    }

    FlowObject* find(const std::string& targetName);
};

}  // namespace kine
