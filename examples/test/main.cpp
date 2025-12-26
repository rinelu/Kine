#include <kine.hpp>
#include <math.hpp>

using namespace kine;

struct Transform
{
    vec2 pos{0.0f};
};

struct Velocity
{
    vec2 vel{0.0f};
};

struct Sprite
{
    vec2 size{32.0f, 32.0f};
    std::array<float, 4> color{255, 255, 0, 255};
};

constexpr float GRAVITY = 900.0f;
constexpr float FLAP_VELOCITY = -350.0f;
constexpr float GROUND_Y = 520.0f;

class Bird : public FlowObject
{
   public:
    void on_attach(ECS& ecs) override
    {
        ecs.add_component<Transform>(entity, vec2{200, 200});
        ecs.add_component<Velocity>(entity);
        ecs.add_component<Sprite>(entity);
    }

    void update(ECS& ecs, float dt) override
    {
        if (ecs.get_context<Input*>()->key_pressed(GLFW_KEY_SPACE))
        {
            ecs.get_component<Velocity>(entity).vel.y = FLAP_VELOCITY;
        }
    }
};

void physics_system(ECS& ecs, float dt, float)
{
    auto view = ecs.view<Transform, Velocity>();

    for (auto e : view)
    {
        auto& t = view.get<Transform>(e);
        auto& v = view.get<Velocity>(e);

        v.vel.y += GRAVITY * dt;
        t.pos += v.vel * dt;

        // ground collision
        if (t.pos.y > GROUND_Y)
        {
            t.pos.y = GROUND_Y;
            v.vel.y = 0;
        }
    }
}

void render_system(ECS& ecs, float, float)
{
    auto& renderer = ecs.get_context<Renderer*>();
    auto& list = ecs.get_context<RenderList*>();

    auto view = ecs.view<Transform, Sprite>();
    for (auto e : view)
    {
        list->clear();
        auto& t = view.get<Transform>(e);
        auto& s = view.get<Sprite>(e);

        list->add_rect(t.pos, s.size, s.color);
    }
}

int main()
{
    KineEngine engine(800, 600, "Kine - Flappy Bird");

    auto* tree = engine.flow_tree();
    tree->create<Bird>("Bird");
    tree->finalize();

    auto* sched = engine.scheduler();

    sched->add_system("Physics", physics_system);
    sched->add_system("Render", render_system);
    sched->add_dependency("Physics", "Render");

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
