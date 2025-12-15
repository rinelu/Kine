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
        vec2 p = vUV - 0.5;
        vec2 px = p * vSize;
        float r = vSize.x * 0.5; // Circle radius in pixels
        float d = length(px);    // Distance from center in pixels
        float alpha = 1.0 - smoothstep(r - 1.0, r, d); 
        FragColor = vec4(vColor.rgb, vColor.a * alpha);

        return;
    }
    
    if (vType < 3.5)
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

    FragColor = vColor;
}
