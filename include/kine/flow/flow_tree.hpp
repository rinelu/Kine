#pragma once
#include "flow_object.hpp"

namespace kine
{

class FlowTree
{
   public:
    ECS ecs;

    FlowTree() = default;
    ~FlowTree() = default;

    /**
     * @brief Create the root node of the tree.
     *
     * @tparam T Type of the root node (must derive from FlowObject)
     * @param name Name of the root node
     * @param args Constructor arguments for T
     *
     * @return Pointer to the root node.
     *
     * Only one root node is supported.
     */
    template <typename T = FlowObject, typename... Args>
    T* create(const std::string& name, Args&&... args)
    {
        static_assert(std::is_base_of<FlowObject, T>::value, "Type T must inherit from FlowObject");

        root = std::make_unique<T>(std::forward<Args>(args)...);
        root->name = name;
        return static_cast<T*>(root.get());
    }

    /**
     * @brief Finalize the tree.
     *
     * This:
     * - Creates ECS entities for all nodes
     * - Calls on_attach() on all nodes
     * - Calls init() on all nodes
     */
    void finalize();

    /**
     * @brief Update the tree.
     *
     * @param dt Delta time in seconds.
     */
    void update(float dt);

    /**
     * @brief Run fixed update on the tree.
     *
     * @param dt Fixed delta time in seconds.
     */
    void fixed_update(float dt);

    void remove_queued_objs();

   private:
    std::unique_ptr<FlowObject> root;
    bool ready = false;

    void attach_recursive(FlowObject*);
    void init_recursive(FlowObject*);
    void update_recursive(FlowObject*, float);
    void fixed_update_recursive(FlowObject*, float);
};

}  // namespace kine
