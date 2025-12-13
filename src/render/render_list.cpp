#include "render_list.hpp"

namespace kine
{

RenderList::RenderList() { commands.reserve(1024); }

void RenderList::add_sprite(const std::string& texture, vec2 pos, float rotation, vec2 pivot,
                           float scale, int32_t layer)
{
    RenderCommand cmd{};
    cmd.type = RenderType::Sprite;
    cmd.texture_name = texture;
    cmd.x = pos.x;
    cmd.y = pos.y;
    cmd.rotation = rotation;
    cmd.pivotX = pivot.x;
    cmd.pivotY = pivot.y;
    cmd.scale = scale;
    cmd.layer = layer;

    commands.push_back(cmd);
}

void RenderList::add_rect(vec2 pos, vec2 size, std::array<float, 4> color, float scale, int32_t layer)
{
    RenderCommand cmd{};
    cmd.type = RenderType::Rect;
    cmd.x = pos.x;
    cmd.y = pos.y;
    cmd.width = size.x;
    cmd.height = size.y;
    cmd.scale = scale;
    cmd.color = color;
    cmd.layer = layer;

    commands.push_back(cmd);
}

void RenderList::add_circle(vec2 pos, float radius, std::array<float, 4> color, float scale, int32_t layer)
{
    RenderCommand cmd{};
    cmd.type = RenderType::Circle;
    cmd.x = pos.x;
    cmd.y = pos.y;
    cmd.radius = radius;
    cmd.scale = scale;
    cmd.color = color;
    cmd.layer = layer;

    commands.push_back(cmd);
}

void RenderList::add_line(vec2 pos1, vec2 pos2, float thickness, std::array<float, 4> color,
                         float scale, int32_t layer)
{
    RenderCommand cmd{};
    cmd.type = RenderType::Line;
    cmd.x = pos1.x;
    cmd.y = pos1.y;
    cmd.x2 = pos2.x;
    cmd.y2 = pos2.y;
    cmd.radius = thickness;
    cmd.scale = scale;
    cmd.color = color;
    cmd.layer = layer;

    commands.push_back(cmd);
}

}  // namespace kine
