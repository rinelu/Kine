#include "flow/flow_tree.hpp"
#include "flow/flow_object.hpp"

namespace kine
{

void FlowTree::finalize()
{
    if (!root) return;
    attach_recursive(root.get());
    init_recursive(root.get());

    ready = true;
}

void FlowTree::update(float dt)
{
    if (!root) return;
    update_recursive(root.get(), dt);
}

void FlowTree::fixed_update(float fixed_dt)
{
    if (!root) return;
    fixed_update_recursive(root.get(), fixed_dt);
}

void FlowTree::attach_recursive(FlowObject* obj)
{
    obj->entity = ecs_.create();
    obj->on_attach();

    for (auto& c : obj->children) attach_recursive(c.get());
}

void FlowTree::init_recursive(FlowObject* obj)
{
    obj->init();
    for (auto& c : obj->children) init_recursive(c.get());
}

void FlowTree::update_recursive(FlowObject* obj, float dt)
{
    obj->update(dt);
    for (auto& c : obj->children) update_recursive(c.get(), dt);
}

void FlowTree::fixed_update_recursive(FlowObject* obj, float fixed_dt)
{
    obj->fixed_update(fixed_dt);
    for (auto& c : obj->children) fixed_update_recursive(c.get(), fixed_dt);
}
}  // namespace kine
