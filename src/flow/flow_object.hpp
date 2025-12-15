#pragma once
#include <memory>
#include <string>
#include "../ecs/ecs.hpp"

namespace kine
{

class FlowObject
{
   public:
    FlowObject() = default;
    virtual ~FlowObject() = default;

    std::string name = "";

    FlowObject* parent = nullptr;
    std::vector<std::unique_ptr<FlowObject>> children;
    ECS ecs;

    virtual void on_attach() {}
    virtual void on_detach() {}
    virtual void init() {}
    virtual void update(float dt) { (void) dt; }
    virtual void tick(float dt) { (void) dt; }

    template <typename T, typename... Args>
    T* add_child(const std::string& childName, Args&&... args)
    {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->name = childName;
        child->parent = this;

        T* raw = child.get();
        children.push_back(std::move(child));

        raw->on_mount();
        return raw;
    }

    FlowObject* find(const std::string& targetName);
};

}  // namespace sf
