#pragma once
#include <vector>

#include "kine/math.hpp"
#include "render_command.hpp"

namespace kine::render
{

inline std::vector<RenderCommand> commands;
inline bool initialized;

void init();

inline RenderCommand base_cmd(RenderType type, vec2 pos, float scale, int32_t layer)
{
    RenderCommand cmd{};
    cmd.type = type;
    cmd.x = pos.x;
    cmd.y = pos.y;
    cmd.scale = scale;
    cmd.layer = layer;
    return cmd;
}
inline const std::vector<RenderCommand>& get() { return commands; }

inline void clear() { commands.clear(); }

void draw_sprite(const std::string& texture_name, vec2 pos, float rotation, vec2 pivot, float scale = 1,
                 int32_t layer = 1);

void draw_rect(vec2 pos, vec2 size, std::array<float, 4> color = {255, 255, 255, 255}, float scale = 1,
               int32_t layer = 1);

void draw_circle(vec2 pos, float radius, std::array<float, 4> color = {255, 255, 255, 255}, float scale = 1,
                 int32_t layer = 1);

void draw_line(vec2 pos_start, vec2 pos_end, float thickness, std::array<float, 4> color = {255, 255, 255, 255},
               float scale = 1, int32_t layer = 1);
void draw_text(Font* font, const std::string& text, vec2 pos, float rotation, vec2 pivot,
               std::array<float, 4> color = {255, 255, 255, 255}, float scale = 1, int32_t layer = 1);

}  // namespace kine::render
