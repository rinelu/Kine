#include "kine.hpp"
#include <memory>

KineEngine::KineEngine() = default;
KineEngine::~KineEngine() = default;

void KineEngine::init(int width, int height, const char* title)
{
    time = std::make_unique<Time>();
    ecs.create();

    window = std::make_shared<Window>(width, height, title);
    resourceManager = std::make_shared<ResourceManager>();
    render_list = std::make_shared<RenderList>();
    renderer = std::make_unique<Renderer>(resourceManager.get(), render_list.get(), window.get()->get());

    ecs.set_context<RenderList&>(*render_list);
    ecs.set_context<ResourceManager&>(*resourceManager);

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
}


void KineEngine::shutdown()
{
    resourceManager->shutdown();
    renderer->shutdown();
}
