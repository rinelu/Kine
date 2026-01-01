#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "kine/ecs/ecs.hpp"

namespace kine::scheduler
{

using SystemFunc = std::function<void(ECS&, float, float)>;

// Registered systems
inline std::unordered_map<std::string, SystemFunc> systems;

// Dependencies: before - after
inline std::unordered_map<std::string, std::vector<std::string>> edges;

// Sorted execution list
inline std::vector<std::string> sorted;

// Permanently visited in DFS
inline std::unordered_map<std::string, bool> perm;

// Temporarily visited in DFS
inline std::unordered_map<std::string, bool> temp;

inline bool dirty = true;
inline bool has_cycle = false;

void reset();
void init();
void shutdown();

bool add_system(std::string name, SystemFunc func);
bool add_dependency(const std::string& before, const std::string& after);

bool rebuild_order();

void update(ECS& ecs, float dt, float alpha);
void fixed_update(ECS& ecs, float& accumulator, float fixedDt, float alpha);

// private:
bool visit(const std::string& name);

}  // namespace kine::scheduler
