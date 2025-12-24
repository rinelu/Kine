#include "kine.hpp"
#include <memory>

KineEngine::KineEngine(int width, int height, const char* title)
{
    time = std::make_unique<Time>();

    window = std::make_shared<Window>(width, height, title);
    _input = std::make_shared<Input>();
    _resourceManager = std::make_shared<ResourceManager>();
    _render_list = std::make_shared<RenderList>();
    _renderer = std::make_unique<Renderer>(_resourceManager.get(), _render_list.get(), window.get()->get());
    flow = std::make_shared<FlowTree>();
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
    float dt = time->dt;

    flow->update(dt);
}

void KineEngine::render_frame() { _renderer->render(); }

void KineEngine::shutdown()
{
    _resourceManager->shutdown();
    _renderer->shutdown();
}
