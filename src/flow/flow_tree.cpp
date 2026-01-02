#include "kine/flow/flow_tree.hpp"
#include "kine/flow/flow_object.hpp"

namespace kine
{

void FlowTree::finalize()
{
    if (!root || ready) return;
    attach_recursive(root.get());
    init_recursive(root.get());

    ready = true;
}

void FlowTree::update(float dt)
{
    if (!ready) return;
    update_recursive(root.get(), dt);
}

void FlowTree::fixed_update(float fixed_dt)
{
    if (!ready) return;
    fixed_update_recursive(root.get(), fixed_dt);
}

void FlowTree::attach_recursive(FlowObject* obj)
{
    obj->entity = ecs.create();
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
    if (!obj->enabled) return;
    if (!obj->pause_mode) obj->update(dt);
    for (auto& c : obj->children) update_recursive(c.get(), dt);
}

void FlowTree::fixed_update_recursive(FlowObject* obj, float dt)
{
    if (!obj->enabled) return;
    if (!obj->pause_mode) obj->fixed_update(dt);
    for (auto& c : obj->children) fixed_update_recursive(c.get(), dt);
}

void FlowTree::remove_queued_objs()
{
    std::function<void(FlowObject*, FlowObject*)> recurse = [&](FlowObject*, FlowObject* node)
    {
        if (!node) return;

        auto it = node->children.begin();
        while (it != node->children.end())
        {
            FlowObject* child = it->get();
            if (child->queued_for_deletion)
            {
                it = node->children.erase(it);
                continue;
            }
            recurse(node, child);
            ++it;
        }
    };

    recurse(nullptr, root.get());
}

}  // namespace kine
