#include "flow_tree.hpp"

namespace kine
{

void FlowTree::finalize()
{
    if (!root) return;
    call_init(root.get());
    ready_called = true;
}

void FlowTree::call_init(FlowObject* obj)
{
    obj->init();
    for (auto& c : obj->children) call_init(c.get());
}

void FlowTree::update(float dt)
{
    if (!root) return;
    if (!ready_called) finalize();

    call_update(root.get(), dt);
}

void FlowTree::call_update(FlowObject* obj, float dt)
{
    obj->tick(dt);
    for (auto& c : obj->children) call_update(c.get(), dt);
}

}  // namespace kine
