#pragma once
#include <memory>

#include "core/time.hpp"
#include "ecs/ecs.hpp"
#include "flow/flow_tree.hpp"
#include "render/renderer.hpp"
#include "render/window.hpp"
#include "resources/resource_manager.hpp"

using namespace kine;

class KineEngine
{
   public:
    bool running = false;
    ECS ecs;

    KineEngine();
    ~KineEngine();

    void init();
    void shutdown();
    void begin_frame();
    void update();
    void render_frame();

    float deltaTime() const { return time->dt; }
    FlowTree& flow_tree() { return *flow; }

   private:
    std::unique_ptr<Time> time;
    std::unique_ptr<FlowTree> flow;

    std::shared_ptr<Window> window;
    std::shared_ptr<ResourceManager> resourceManager;
    std::shared_ptr<RenderList> render_list;
    std::shared_ptr<Renderer> renderer;
};
