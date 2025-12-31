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
