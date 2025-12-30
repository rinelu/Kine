#include "render/render_list.hpp"
#include "log.h"

namespace kine::render
{

void init()
{
    initialized = true;
    commands.reserve(1024);
}
void shutdown()
{
    if (!initialized) return;
    commands.clear();
    initialized = false;
}

bool is_initialized()
{
    if (!initialized)
    {
        LOG_ERROR("RenderList: RenderList is not initialized. Perhaps you forgot to initialized the engine.");
        return false;
    }
    return true;
}

void draw_sprite(const std::string& texture, vec2 pos, float rotation, vec2 pivot, float scale, int32_t layer)
{
    if (!is_initialized()) return;
    RenderCommand cmd = base_cmd(RenderType::Sprite, pos, scale, layer);
    cmd.texture_name = texture;
    cmd.rotation = rotation;
    cmd.pivotX = pivot.x;
    cmd.pivotY = pivot.y;
    cmd.color = {255, 255, 255, 255};

    commands.push_back(cmd);
}

void draw_rect(vec2 pos, vec2 size, std::array<float, 4> color, float scale, int32_t layer)
{
    if (!is_initialized()) return;
    RenderCommand cmd = base_cmd(RenderType::Rect, pos, scale, layer);
    cmd.width = size.x;
    cmd.height = size.y;
    cmd.color = color;

    commands.push_back(cmd);
}

void draw_circle(vec2 pos, float radius, std::array<float, 4> color, float scale, int32_t layer)
{
    if (!is_initialized()) return;
    RenderCommand cmd = base_cmd(RenderType::Circle, pos, scale, layer);
    cmd.radius = radius;
    cmd.color = color;

    commands.push_back(cmd);
}

void draw_line(vec2 pos1, vec2 pos2, float thickness, std::array<float, 4> color, float scale, int32_t layer)
{
    if (!is_initialized()) return;
    RenderCommand cmd = base_cmd(RenderType::Line, pos1, scale, layer);
    cmd.x2 = pos2.x;
    cmd.y2 = pos2.y;
    cmd.radius = thickness;
    cmd.color = color;

    commands.push_back(cmd);
}

void draw_text(Font* font, const std::string& text, vec2 pos, float rotation, vec2 pivot, std::array<float, 4> color,
               float scale, int32_t layer)
{
    if (!is_initialized()) return;
    RenderCommand cmd = base_cmd(RenderType::Text, pos, scale, layer);
    cmd.font = font;
    cmd.text = text;
    cmd.rotation = rotation;
    cmd.pivotX = pivot.x;
    cmd.pivotY = pivot.y;
    cmd.color = color;

    commands.push_back(cmd);
}

}  // namespace kine::render
