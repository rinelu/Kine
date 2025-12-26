#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec2 aOrigin;
layout (location = 4) in vec2 aSize;
layout (location = 5) in vec2 aMisc;
layout (location = 6) in float aRotation;
layout (location = 7) in float aType;

uniform mat4 uProjection;

out vec2 vSize;
out vec2 vUV;
out vec4 vColor;
flat out float vType;

void main()
{
    vUV    = aUV;
    vColor = aColor;
    vType  = aType;
    vSize  = aSize;

    gl_Position = uProjection * vec4(aPos.xy, 0.0, 1.0);
}
