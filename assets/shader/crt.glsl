#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform vec2 resolution;
uniform int frame;

vec2 barrelDistort(vec2 uv, float strength) {
    vec2 centered = uv * 2.0 - 1.0;

    float aspect = min(resolution.y / resolution.x, 0.5);
    float r2 = centered.x * centered.x * aspect + centered.y * centered.y;

    centered *= 1.0 + strength * r2;

    float edgeR2 = 1.0 * aspect;
    float edgeScale = 1.0 + strength * edgeR2;
    centered /= edgeScale;

    return centered * 0.5 + 0.5;
}

void main()
{
    vec2 uv = barrelDistort(fragTexCoord, 0.02);

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float offset = 0.001 * (1920 / resolution.x) * min(resolution.y / 1080, 1);

    float r = texture(texture0, uv + vec2(offset, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(offset, 0.0)).b;

    vec3 color = vec3(r, g, b);

    float scan = sin(uv.y * resolution.y * 1.5) * 0.1;

    float flicker = 0.95 + 0.05 * sin(time * 100.0);

    color *= flicker;
    color -= scan;

    float vigUV = uv.x * (1.0 - uv.x) * uv.y * (1.0 - uv.y);
    float vignette = pow(vigUV * 16.0, 0.2);
    color *= vignette;

    finalColor = vec4(color, 1.0);
}