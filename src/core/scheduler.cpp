#include "core/scheduler.hpp"

namespace kine
{

bool Scheduler::add_system(std::string name, SystemFunc func)
{
    systems.emplace(std::move(name), std::move(func));
    dirty = true;
    return true;
}

bool Scheduler::add_dependency(const std::string& before, const std::string& after)
{
    if (!systems.contains(before) || !systems.contains(after)) return false;

    edges[before].push_back(after);
    dirty = true;
    return true;
}

bool Scheduler::visit(const std::string& name)
{
    if (perm[name]) return true;
    if (temp[name])
    {
        has_cycle = true;
        return false;
    }

    temp[name] = true;

    for (const auto& dep : edges[name])
        if (!visit(dep)) return false;

    temp[name] = false;
    perm[name] = true;
    sorted.push_back(name);

    return true;
}

bool Scheduler::rebuild_order()
{
    sorted.clear();
    perm.clear();
    temp.clear();
    has_cycle = false;

    for (auto& [name, _] : systems)
    {
        if (!perm[name] && !visit(name))
        {
            sorted.clear();
            return false;  // cycle error
        }
    }

    dirty = false;
    return true;
}

void Scheduler::update(ECS& ecs, float dt, float alpha)
{
    if (dirty && !rebuild_order()) return;  // safe fail: skip update

    for (auto& name : sorted) systems[name](ecs, dt, alpha);
}

void Scheduler::fixed_update(ECS& ecs, float& accumulator, float fixed_dt, float alpha)
{
    if (dirty && !rebuild_order()) return;  // safe fail: skip update

    while (accumulator >= fixed_dt)
    {
        for (auto& name : sorted) systems[name](ecs, fixed_dt, alpha);
        accumulator -= fixed_dt;
    }
}

}  // namespace kine
