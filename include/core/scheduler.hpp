#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ecs/ecs.hpp"

namespace kine
{

class Scheduler
{
   public:
    using SystemFunc = std::function<void(ECS&, float, float)>;

    bool add_system(std::string name, SystemFunc func);
    bool add_dependency(const std::string& before, const std::string& after);

    bool rebuild_order();

    void update(ECS& ecs, float dt, float alpha);
    void fixed_update(ECS& ecs, float& accumulator, float fixedDt, float alpha);

   private:
    bool visit(const std::string& name);

    // Registered systems
    std::unordered_map<std::string, SystemFunc> systems;

    // Dependencies
    std::unordered_map<std::string, std::vector<std::string>> edges;

    // Sorted execution list
    std::vector<std::string> sorted;

    // Permanently visited in DFS
    std::unordered_map<std::string, bool> perm;

    // Temporarily visited in DFS
    std::unordered_map<std::string, bool> temp;

    bool dirty = true;  ///< If true, ordering must be rebuilt
    bool has_cycle = false;
};

}  // namespace kine
