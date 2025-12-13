#include "kine.hpp"

int main(void)
{
    KineEngine engine;
    engine.init();

    while (engine.running)
    {
        engine.begin_frame();

        auto& list = engine.ecs.get_context<RenderList>();
        list.add_rect(vec2(10, 10), vec2(10, 10), {255, 0, 0, 255});
        // list.add_line(vec2(130, 130), vec2(30, 80), 10, {255, 0, 0, 255});
        list.add_circle(vec2(500, 20), 40, {255, 0, 0, 255});

        engine.update();
        engine.render_frame();
    }
    engine.shutdown();
    return 0;
}
