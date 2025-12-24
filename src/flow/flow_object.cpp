#include "flow/flow_object.hpp"

namespace kine
{
FlowObject* FlowObject::find(const std::string& targetName)
{
    for (auto& c : children)
    {
        if (c->name == targetName) return c.get();

        if (auto* nested = c->find(targetName)) return nested;
    }
    return nullptr;
}

}  // namespace kine
