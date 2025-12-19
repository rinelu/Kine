#pragma once
#include <unordered_map>
#include "math.hpp"
#include "render/texture2d.hpp"

namespace kine
{
struct Glyph
{
    vec2 size;
    vec2 bearing;
    float advance;
    vec2 uv[4];
};

struct Font
{
    Texture2D* texture;
    std::unordered_map<char, Glyph> glyphs;
    float line_height;
    float ascent;
};
}  // namespace kine
