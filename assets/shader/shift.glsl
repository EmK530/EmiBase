#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;
uniform vec2 resolution;
uniform int frame;

uniform float intensity;

float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float randLine(float lineId, float frame)
{
    return hash12(vec2(lineId, frame));
}

void main()
{
    vec2 uv = fragTexCoord;

    float groups = 360;
    float lineId = floor(uv.y * groups);

    float r = randLine(lineId, frame);

    float offset = (r - 0.5) * intensity * (1920 / resolution.x);

    vec2 shiftedUV = uv + vec2(offset, 0.0);

    vec3 col = texture(texture0, shiftedUV).rgb;

    finalColor = vec4(col, 1.0);
}