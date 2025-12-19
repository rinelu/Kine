#include "kine.hpp"

int main(void)
{
    KineEngine engine = {1280, 720, "Test"};
    engine.get_renderer()->set_virtual_resolution(640, 380);
    engine.init();
    auto& resource = engine.ecs.get_context<ResourceManager>();
    auto& font = resource.fonts().load("roboto", "fonts/Roboto/Roboto-Medium.ttf", 64);

    while (engine.running)
    {
        engine.begin_frame();

        auto& list = engine.ecs.get_context<RenderList>();
        list.clear();

        // Solid black background (fills the entire FBO)
        list.add_rect(vec2(0, 0), vec2(1280, 720), {20, 20, 20, 255}, 1, 0);

        // Thick white border (5 pixels thick)
        list.add_line(vec2(5, 5), vec2(1275, 5), 5.0f, {255, 255, 255, 255}, 1, 0);       // Top
        list.add_line(vec2(1275, 5), vec2(1275, 715), 5.0f, {255, 255, 255, 255}, 1, 0);  // Right

        // Large Blue Header Bar at the top
        list.add_rect(vec2(50, 50), vec2(1180, 100), {0, 100, 200, 255}, 1, 1);

        // Smaller Red Data Box
        list.add_sprite("error", vec2(300, 450), 0, {0, 0});
        list.add_text(&font, "Hello, World!", vec2(200, 100), 0, vec2(0, 0));

        // Central Green Circle (Radius 100)
        list.add_circle(vec2(640, 360), 100.0f, {0, 255, 0, 255}, 1, 2);

        // Small Yellow "Indicator" Circle (Radius 20)
        list.add_circle(vec2(900, 600), 20.0f, {255, 255, 0, 255}, 1, 2);

        // Thick Diagonal Line (connecting the blue bar to the yellow circle)
        list.add_line(vec2(100, 150), vec2(900, 600), 15.0f, {255, 150, 0, 255}, 1, 3);  // Orange, very thick

        // Thin Vertical Line on the right side
        list.add_line(vec2(1050, 50), vec2(1050, 650), 2.0f, {0, 255, 255, 255}, 1, 3);  // Cyan, thin

        engine.update();
        engine.render_frame();
    }
    engine.shutdown();
    return 0;
}
