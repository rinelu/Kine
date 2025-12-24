#pragma once
#include <memory>

#include "core/time.hpp"
#include "flow/flow_tree.hpp"
#include "io/input.hpp"
#include "render/renderer.hpp"
#include "render/window.hpp"
#include "resources/resource_manager.hpp"

using namespace kine;

class KineEngine
{
   public:
    bool running = false;

    KineEngine(int width, int height, const char* title);
    ~KineEngine();

    void init();
    void shutdown();
    void begin_frame();
    void update();
    void render_frame();

    inline float delta_time() const { return time->dt; }
    inline Renderer* renderer() const { return _renderer.get(); }
    inline Input* input() const { return _input.get(); }
    inline ResourceManager* resource() const { return _resourceManager.get(); }
    inline RenderList* render() const { return _render_list.get(); }
    inline FlowTree& flow_tree() { return *flow; }

   private:
    std::unique_ptr<Time> time;
    std::unique_ptr<Renderer> _renderer;

    std::shared_ptr<FlowTree> flow;
    std::shared_ptr<Window> window;
    std::shared_ptr<Input> _input;
    std::shared_ptr<ResourceManager> _resourceManager;
    std::shared_ptr<RenderList> _render_list;
};
