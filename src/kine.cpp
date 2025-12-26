#include "kine.hpp"
#include <memory>

KineEngine::KineEngine(int width, int height, const char* title)
{
    time = std::make_unique<Time>();
    _scheduler = std::make_unique<Scheduler>();

    window = std::make_shared<Window>(width, height, title);
    _input = std::make_shared<Input>();
    _resourceManager = std::make_shared<ResourceManager>();
    _render_list = std::make_shared<RenderList>();
    _renderer = std::make_unique<Renderer>(_resourceManager.get(), _render_list.get(), window.get()->get());

    flow = std::make_shared<FlowTree>();
    flow->ecs().set_context<Input*>(input());
    flow->ecs().set_context<Renderer*>(renderer());
    flow->ecs().set_context<RenderList*>(render());
}
KineEngine::~KineEngine() = default;

void KineEngine::init()
{
    _resourceManager->init();
    _renderer->init();
    _input->init(window->get());
    running = true;
}

void KineEngine::begin_frame()
{
    time->begin_frame();
    _input->begin_frame();
    if (window->should_close()) running = false;

    glfwPollEvents();
}

void KineEngine::update()
{
    float dt = delta_time();

    flow->update(dt);

    _scheduler->update(flow->ecs(), dt, time->alpha);

    _scheduler->fixed_update(flow->ecs(), time->accumulator, time->fixed_dt, time->alpha);
}

void KineEngine::render_frame() { _renderer->render(); }

void KineEngine::shutdown()
{
    _resourceManager->shutdown();
    _renderer->shutdown();
}
