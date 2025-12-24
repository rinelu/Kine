#include "kine.hpp"

int main(void)
{
    KineEngine engine = {1280, 720, "Wow"};
    engine.renderer()->set_virtual_resolution(640, 380);

    auto* root = engine.flow_tree().create("root");
    root->add_child<FlowObject>("Hello, World");

    engine.init();
    while (engine.running)
    {
        engine.begin_frame();

        engine.update();
        engine.render_frame();
    }
    engine.shutdown();
    return 0;
}
