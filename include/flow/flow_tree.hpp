#pragma once
#include "flow_object.hpp"

namespace kine
{

class FlowTree
{
   public:
    FlowTree() = default;

    template <typename T = FlowObject, typename... Args>
    T* create(const std::string& name, Args&&... args)
    {
        root = std::make_unique<T>(std::forward<Args>(args)...);
        root->name = name;

        LOG_DEBUG("FlowTree: Creating root ", name);
        return static_cast<T*>(root.get());
    }

    void finalize();
    void update(float dt);

    ECS& ecs() { return ecs_; }

   private:
    std::unique_ptr<FlowObject> root;
    ECS ecs_;
    bool ready = false;

    void attach_recursive(FlowObject* obj);
    void init_recursive(FlowObject* obj);
    void update_recursive(FlowObject* obj, float dt);
};

}  // namespace kine
