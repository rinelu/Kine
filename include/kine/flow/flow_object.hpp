#pragma once
#include <string>
#include <unordered_set>
#include <vector>

#include "kine/ecs/ecs.hpp"

namespace kine
{

class FlowTree;

class FlowObject
{
   public:
    virtual ~FlowObject();

    std::string name;
    FlowObject* parent = nullptr;
    std::vector<std::unique_ptr<FlowObject>> children;
    std::unordered_set<std::string> groups;

    bool enabled = true;
    bool pause_mode = false;

    bool queued_for_deletion = false;

    Entity entity;

    /**
     * @brief Called when the node is attached to a FlowTree.
     *
     * Use this to add ECS components.
     * At this point:
     * - The ECS Entity is valid
     * - Parent and children relationships are established
     */
    virtual void on_attach() {}

    /**
     * @brief Called when the node is detached or destroyed.
     *
     * Called before the ECS Entity is destroyed.
     */
    virtual void on_detach() {}

    /**
     * @brief Called once after the entire tree has been attached.
     *
     * Called exactly once per node.
     */
    virtual void init() {}

    /**
     * @brief Called every frame.
     *
     * @param dt Delta time in seconds.
     */
    virtual void update(float) {}

    /**
     * @brief Called at a fixed timestep.
     *
     * @param dt Fixed delta time in seconds.
     */
    virtual void fixed_update(float) {}

    template <typename T, typename... Args>
    T* add_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<FlowObject, T>, "T must derive from FlowObject");

        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->parent = this;

        T* raw = child.get();
        children.push_back(std::move(child));
        return raw;
    }

    void remove_child(FlowObject* child, bool free_memory = true);
    void reparent(FlowObject* new_parent);

    FlowObject* find(const std::string& path);
    FlowObject* find_or_null(const std::string& path);

    template <typename T>
    void find_all(std::vector<T*>& out)
    {
        static_assert(std::is_base_of_v<FlowObject, T>, "T must derive from FlowObject");
        if (auto* self = dynamic_cast<T*>(this)) out.push_back(self);
        for (auto& child : children) child->find_all<T>(out);
    }

    template <typename T>
    T* find_type()
    {
        if (auto* self = dynamic_cast<T*>(this)) return self;

        for (auto& c : children)
            if (auto* found = c->find_type<T>()) return found;

        return nullptr;
    }

    void add_to_group(const std::string& group);
    void remove_from_group(const std::string& group);
    bool is_in_group(const std::string& group) const;

    void queue_free();

   private:
    friend class FlowTree;
};

}  // namespace kine
