#pragma once
#include <math.hpp>
#include <vector>
#include "render_command.hpp"

namespace kine
{
class RenderList
{
   public:
    RenderList();

    const std::vector<RenderCommand>& get() const { return commands; }

    void clear() { commands.clear(); }

    void add_sprite(const std::string& texture_name, vec2 pos, float rotation, vec2 pivot,
                   float scale = 1, int32_t layer = 1);

    void add_rect(vec2 pos, vec2 size, std::array<float, 4> color, float scale = 1, int32_t layer = 1);

    void add_circle(vec2 pos, float radius, std::array<float, 4> color, float scale = 1, int32_t layer = 1);

    void add_line(vec2 pos_start, vec2 pos_end, float thickness, std::array<float, 4> color, float scale = 1,
                 int32_t layer = 1);

   private:
    std::vector<RenderCommand> commands;
};

}  // namespace kine
