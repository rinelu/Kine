#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
layout (location = 7) in float aType;

uniform mat4 uProjection;

out vec2 vUV;
out vec4 vColor;
flat out float vType;

void main()
{
    vUV    = aUV;
    vColor = aColor;
    vType  = aType;

    gl_Position = uProjection * vec4(aPos.xy, 0.0, 1.0);
}
