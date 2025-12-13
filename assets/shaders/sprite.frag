#version 330 core

in vec2 vUV;
in vec4 vColor;
flat in float vType;

out vec4 FragColor;

uniform float uAspect;
uniform sampler2D uTexture;

void main()
{
    if (vType < 0.5)
    {
        FragColor = texture(uTexture, vUV) * vColor;
        return;
    }

    if (vType < 1.5)
    {
        FragColor = vColor;
        return;
    }

    if (vType < 2.5)
    {
        vec2 p = vUV * 2.0 - 1.0;
        float d = dot(p, p);

        if (d > 1.0)
            discard;

        FragColor = vColor;
        return;
    }

    FragColor = vColor;
}
