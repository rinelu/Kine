#include <kine.hpp>
#include <math.hpp>
#include "render/render_list.hpp"

using kine::ECS;
using kine::FlowObject;
using kine::scheduler::add_dependency;
using kine::scheduler::add_system;

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

struct Collider
{
    vec2 size;
};

struct PipeConfig
{
    float speed = 200.0f;
};

#define SCREEN_WIDTH 800.0f
#define SCREEN_HEIGHT 600.0f

#define PIPE_MARGIN 0.0f  // space above/below
#define PIPE_SPACING 500.0f
#define PIPE_COUNT 4

#define GRAVITY 900.0f
#define FLAP_VELOCITY -350.0f
#define GROUND_Y 520.0f

class Bird : public FlowObject
{
   public:
    void on_attach() override
    {
        kine::Requires<Velocity, Sprite>::attach(entity);
        entity.add<Transform>(vec2{200, 200});
    }

    void update(float dt) override
    {
        if (kine::input::key_pressed(&kine::global_input, GLFW_KEY_SPACE)) entity.get<Velocity>().vel.y = FLAP_VELOCITY;
    }

    void fixed_update(float dt) override
    {
        auto& pos = entity.get<Transform>().pos;
        auto& vel = entity.get<Velocity>().vel;

        vel.y += GRAVITY * dt;
        pos += vel * dt;

        // ground collision
        if (pos.y > GROUND_Y)
        {
            pos.y = GROUND_Y;
            vel.y = 0;
        }
    }
};

class Pipe : public FlowObject
{
   public:
    float spawn_x;
    float center_y;

    kine::Entity top;
    kine::Entity bottom;

    float gap = 160.0f;
    float width = 60.0f;

    Pipe(vec2 pos) : spawn_x(pos.x), center_y(pos.y) {}

    void on_attach() override
    {
        ECS* ecs = entity.get_ecs();

        // create top pipe
        top = ecs->create();
        kine::Requires<Transform, Sprite, Collider>::attach(top);
        top.add<PipeConfig>();

        // create bottom pipe
        bottom = ecs->create();
        kine::Requires<Transform, Sprite, Collider>::attach(bottom);
        bottom.add<PipeConfig>();

        reset();
    }

    void fixed_update(float dt) override
    {
        auto& pos = top.get<Transform>().pos;

        if (pos.x + width < 0.0f)
        {
            spawn_x = pos.x + (PIPE_COUNT * PIPE_SPACING);
            center_y = random_range(150.0f, 450.0f);
            reset();
        }
    }

    void reset()
    {
        // spawn_x = SCREEN_WIDTH;
        center_y = std::clamp(center_y, gap * 0.5f + 20.0f, SCREEN_HEIGHT - gap * 0.5f - 20.0f);

        float gap_top = center_y - gap * 0.5f;
        float gap_bottom = center_y + gap * 0.5f;

        top.get<Transform>().pos = {spawn_x, 0.0f};
        top.get<Sprite>().size = {width, gap_top};
        top.get<Sprite>().color = {0, 255, 0, 255};

        bottom.get<Transform>().pos = {spawn_x, gap_bottom};
        bottom.get<Sprite>().size = {width, SCREEN_HEIGHT - gap_bottom};
        bottom.get<Sprite>().color = {0, 255, 0, 255};

        top.get<PipeConfig>().speed = 200.0f;
        bottom.get<PipeConfig>().speed = 200.0f;
    }
};

void physics_system(ECS& ecs, float dt, float) {}

void pipe_scroll_system(ECS& ecs, float dt, float)
{
    auto view = ecs.view<Transform, PipeConfig>();

    for (auto e : view)
    {
        auto& pos = e.get<Transform>().pos;
        auto& config = e.get<PipeConfig>();

        pos.x -= config.speed * dt;
    }
}

void render_system(ECS& ecs, float, float)
{
    auto& renderer = kine::renderer;
    auto view = ecs.view<Transform, Sprite>();

    for (auto e : view)
    {
        auto& t = e.get<Transform>();
        auto& s = e.get<Sprite>();

        kine::render::draw_rect(t.pos, s.size, s.color);
        // kine::render::draw_text(font, "Score: " + count, vec2 pos, float rotation, vec2 pivot)
    }
}

bool aabb(vec2 p1, vec2 s1, vec2 p2, vec2 s2)
{
    return p1.x < p2.x + s2.x && p1.x + s1.x > p2.x && p1.y < p2.y + s2.y && p1.y + s1.y > p2.y;
}

void collision_system(ECS& ecs, Bird* bird, std::vector<Pipe*> pipes)
{
    auto& bp = bird->entity.get<Transform>().pos;
    auto& bs = bird->entity.get<Sprite>().size;

    for (auto* pipe : pipes)
    {
        auto& ptp = pipe->top.get<Transform>().pos;
        auto& pts = pipe->top.get<Sprite>().size;

        auto& pbp = pipe->bottom.get<Transform>().pos;
        auto& pbs = pipe->bottom.get<Sprite>().size;

        if (aabb(bp, bs, pbp, pbs) || aabb(bp, bs, ptp, pts)) LOG_INFO("Bird is ded");
    }
}

int main()
{
    kine::create(SCREEN_WIDTH, SCREEN_HEIGHT, "Kine - Flappy Bird");

    auto* flow_tree = kine::flow_tree;
    auto& ecs = flow_tree->ecs();
    auto* tree = flow_tree->create<FlowObject>("root");

    auto* bird = tree->add_child<Bird>("Bird");

    float next_pipe_x = SCREEN_WIDTH;
    for (int i = 0; i < PIPE_COUNT; ++i)
    {
        float y = random_range(150.0f, 450.0f);
        float x = SCREEN_WIDTH + (i * PIPE_SPACING);

        tree->add_child<Pipe>("Pipe " + std::to_string(i), vec2(x, y));
    }

    flow_tree->finalize();

    std::vector<Pipe*> pipes;
    tree->find_all<Pipe>(pipes);

    add_system("Physics", physics_system);
    add_system("Render", render_system);
    add_system("PipeScroll", pipe_scroll_system);

    add_dependency("Physics", "PipeScroll");
    add_dependency("PipeScroll", "Render");
    add_dependency("Physics", "Render");

    kine::init();
    while (kine::running)
    {
        kine::begin_frame();
        kine::update();
        collision_system(ecs, bird, pipes);
        kine::render_frame();

        if (kine::input::key_pressed(&kine::global_input, GLFW_KEY_R))
        {
            // Reset bird
            bird->entity.get<Transform>().pos = {200, 200};
            bird->entity.get<Velocity>().vel = {0, 0};

            for (auto e : ecs.view<Transform, PipeConfig>()) e.get<Transform>().pos.x = 900.0f;

            for (auto* pipe : pipes)
            {
                pipe->center_y = random_range(150.0f, 450.0f);
                pipe->reset();
            }
        };
    }

    kine::shutdown();
    return 0;
}
