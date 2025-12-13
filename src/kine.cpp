#include "kine.hpp"
#include <memory>

KineEngine::KineEngine() = default;
KineEngine::~KineEngine() = default;

void KineEngine::init()
{
    time = std::make_unique<Time>();
    ecs.create();

    window = std::make_shared<Window>(1280, 720, "Dreva");
    resourceManager = std::make_shared<ResourceManager>();
    render_list = std::make_shared<RenderList>();
    renderer = std::make_unique<Renderer>(resourceManager.get(), render_list.get(), window.get()->get());

    ecs.set_context<RenderList&>(*render_list);

    resourceManager->init();
    renderer->init();
    running = true;
}

void KineEngine::begin_frame()
{
    time->begin_frame();
    if (window->should_close()) running = false;
}

void KineEngine::update()
{
}

void KineEngine::render_frame()
{
    renderer->render();
    renderer->end_frame();
}


void KineEngine::shutdown()
{
    resourceManager->shutdown();
    renderer->shutdown();
}
