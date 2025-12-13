#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace kine
{

enum class RenderType : uint8_t
{
    Sprite,
    Rect,
    Circle,
    Line
};

struct RenderCommand
{
    RenderType type{RenderType::Sprite};
    int32_t layer{0};

    std::string texture_name{};

    float x{0.0f};
    float y{0.0f};
    float width{0.0f};
    float height{0.0f};
    float rotation{0.0f};
    float scale{0.0f};

    float pivotX{0.0f};
    float pivotY{0.0f};

    float radius{0.0f};
    float x2{0.0f};
    float y2{0.0f};

    std::array<float, 4> color{255.0f, 255.0f, 255.0f, 255.0f};
};

}  // namespace kine
