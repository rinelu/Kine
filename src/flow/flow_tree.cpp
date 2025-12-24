#include "flow/flow_tree.hpp"

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

void FlowTree::attach_recursive(FlowObject* obj)
{
    obj->entity = ecs_.create();
    obj->on_attach(ecs_);

    for (auto& c : obj->children) attach_recursive(c.get());
}

void FlowTree::init_recursive(FlowObject* obj)
{
    obj->init(ecs_);
    for (auto& c : obj->children) init_recursive(c.get());
}

void FlowTree::update_recursive(FlowObject* obj, float dt)
{
    obj->update(ecs_, dt);
    for (auto& c : obj->children) update_recursive(c.get(), dt);
}
}  // namespace kine
