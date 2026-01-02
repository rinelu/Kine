#include "kine/flow/flow_object.hpp"

namespace kine
{

FlowObject::~FlowObject()
{
    on_detach();
    if (entity) entity.destroy();
}

void FlowObject::remove_child(FlowObject* child, bool free_memory)
{
    auto it = std::find_if(children.begin(), children.end(), [&](auto& u) { return u.get() == child; });

    if (it == children.end()) return;

    if (!free_memory)
    {
        (*it)->parent = nullptr;
    }

    children.erase(it);
}

void FlowObject::reparent(FlowObject* new_parent)
{
    if (parent) parent->remove_child(this, false);

    new_parent->children.push_back(std::unique_ptr<FlowObject>(this));
    parent = new_parent;
}

FlowObject* FlowObject::find(const std::string& path)
{
    if (path.empty()) return nullptr;

    if (path == ".") return this;
    if (path == "..") return parent;

    size_t slash = path.find('/');
    std::string part = path.substr(0, slash);
    std::string rest = slash == std::string::npos ? "" : path.substr(slash + 1);

    for (auto& c : children)
    {
        if (c->name == part) return rest.empty() ? c.get() : c->find(rest);
    }

    return nullptr;
}

FlowObject* FlowObject::find_or_null(const std::string& path)
{
    FlowObject* node = find(path);
    return node ? node : nullptr;
}

void FlowObject::add_to_group(const std::string& group) { groups.insert(group); }
void FlowObject::remove_from_group(const std::string& group) { groups.erase(group); }
bool FlowObject::is_in_group(const std::string& group) const { return groups.count(group); }

void FlowObject::queue_free() { queued_for_deletion = true; }

}  // namespace kine
