#pragma once
#include <string>

namespace kine::renderer2d
{

inline static std::string screen_vert = R"(
#version 330 core

layout(location = 0) in vec2 aPos; layout(location = 1) in vec2 aUV; layout(location = 2) in vec4 aColor;
layout(location = 3) in vec2 aOrigin; layout(location = 4) in vec2 aSize; layout(location = 5) in vec2 aMisc;
layout(location = 6) in float aRotation; layout(location = 7) in float aType;

uniform mat4 uProjection;

out vec2 vSize; out vec2 vUV; out vec4 vColor; flat out float vType;

void main() {
    vUV = aUV;
    vColor = aColor;
    vType = aType;
    vSize = aSize;

    gl_Position = uProjection * vec4(aPos.xy, 0.0, 1.0);
})";

inline static std::string screen_frag = R"(
#version 330 core

in vec2 vSize;
in vec2 vUV;
in vec4 vColor;
flat in float vType;

out vec4 FragColor;

uniform float uAspect;
uniform sampler2D uTexture;

float sdRoundedBox(vec2 p, vec2 b, float r)
{
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

void main()
{
    if (vType == 0)
    {
        FragColor = texture(uTexture, vUV) * vColor;
        return;
    }

    if (vType == 5)
    {
        FragColor = vColor;
        return;
    }

    if (vType == 2)
    {
        vec2 p = vUV - 0.5;
        vec2 px = p * vSize;
        float r = vSize.x * 0.5; // Circle radius in pixels
        float d = length(px);    // Distance from center in pixels
        float alpha = 1.0 - smoothstep(r - 1.0, r, d); 
        FragColor = vec4(vColor.rgb, vColor.a * alpha);

        return;
    }
    
    if (vType == 3)
    {
        vec2 p = vUV - 0.5;
        vec2 px = p * vSize;
        vec2 half_size = vSize * 0.5;
        float cap_radius = half_size.y; 
        vec2 b = vec2(half_size.x - cap_radius, cap_radius);
        float dist = sdRoundedBox(px, b, cap_radius);
        float alpha = 1.0 - smoothstep(0.0, 2.0, dist);
        
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
        
        return;
    }

    if (vType == 4) {
        float a = texture(uTexture, vUV).r;
        FragColor = vec4(vColor.rgb, vColor.a * a);
        return;
    }

    FragColor = vColor;
}
)";

inline static std::string blit_vert = R"(
#version 330 core
layout (location = 0) in vec2 aPos; // -1..1
layout (location = 1) in vec2 aUV;

out vec2 vUV;

uniform vec2 uFinalScale;    // scale factor
uniform vec2 uFinalOffset;   // pixel offset
uniform vec2 uWindowSize;    // framebuffer size in pixels
uniform vec2 uVirtualSize;   // virtual resolution

void main()
{
    vUV = aUV;

    vec2 quad_px = (aPos * 0.5 + 0.5) * uVirtualSize;
    vec2 final_px = quad_px * uFinalScale + uFinalOffset;
    vec2 ndc = (final_px / uWindowSize) * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);
}
)";

inline static std::string blit_frag = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, vUV);
}
)";
}  // namespace kine::renderer2d
