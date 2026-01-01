#include "kine/kine.hpp"

namespace kine
{

void create(int width, int height, const char* title)
{
    scheduler::init();
    render::init();
    window::create(width, height, title);
    resource::create();

    input::create(&global_input);
    renderer2d::create(&renderer);

    flow_tree = new FlowTree();
}

void init()
{
    resource::init();
    renderer2d::init(&renderer);
    input::init(&global_input);

    running = true;
}

void begin_frame()
{
    time::begin_frame();
    input::begin_frame(&global_input);
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

void render_frame() { renderer2d::render(&renderer); }

void shutdown()
{
    input::shutdown(&global_input);
    resource::shutdown();
    renderer2d::shutdown(&renderer);
    scheduler::shutdown();

#define RESET(x) \
    delete x;    \
    x = nullptr;

    RESET(flow_tree);
#undef RESET
}

}  // namespace kine
