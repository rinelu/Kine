#include "kine.hpp"
#include "render/render_list.hpp"

namespace kine
{

void create(int width, int height, const char* title)
{
    scheduler::init();
    render::init();
    window::create(width, height, title);
    input = new Input();
    resource_manager = new ResourceManager();
    renderer = new Renderer(resource_manager, window::get());

    flow_tree = new FlowTree();
}

void init()
{
    resource_manager->init();
    renderer->init();
    input->init(window::get());
    running = true;
}

void begin_frame()
{
    time::begin_frame();
    input->begin_frame();
    if (window::should_close()) running = false;

    glfwPollEvents();
}

void update()
{
    float dt = delta_time();

    flow_tree->update(dt);
    flow_tree->fixed_update(time::fixed_dt);

    scheduler::update(flow_tree->ecs(), dt, time::alpha);
    scheduler::fixed_update(flow_tree->ecs(), time::accumulator, time::fixed_dt, time::alpha);
}

void render_frame() { renderer->render(); }

void shutdown()
{
    resource_manager->shutdown();
    renderer->shutdown();

#define RESET(x) \
    delete x;    \
    x = nullptr;

    RESET(flow_tree);
    RESET(input);
    RESET(resource_manager);
    RESET(renderer);
#undef RESET
}

}  // namespace kine
