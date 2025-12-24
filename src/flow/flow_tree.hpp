#pragma once
#include "flow_object.hpp"

namespace kine
{

class FlowTree
{
   public:
    FlowTree() = default;
    ~FlowTree() = default;

    std::unique_ptr<FlowObject> root;

    template <typename T = FlowObject, typename... Args>
    T* create(const std::string& name, Args&&... args)
    {
        root = std::make_unique<T>(std::forward<Args>(args)...);
        root->name = name;
        LOG_INFO("FlowTree: Creating tree root with name ", name);
        return static_cast<T*>(root.get());
    }

    void finalize();
    void update(float dt);
    void tick(float dt);  // TODO

   private:
    void call_init(FlowObject* obj);
    void call_update(FlowObject* obj, float dt);
    void call_tick(FlowObject* obj, float dt);  // TODO

    bool ready_called = false;
};

}  // namespace kine
