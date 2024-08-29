#version 450

layout(location = 0) in vec2 fragCoords;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
} ubo;

const float maxFadeDistance = 50.0f;

float opacityFalloff;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 coord = fragCoords.xy;
    

    // Compute anti-aliased world-space grid lines
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);

    // Just visualize the grid lines directly
    float color = 1.0 - min(line, 1.0);
    outColor = vec4(vec3(color), 0.25);
    
    // Draw the X and Y axis on specific colors.
    if (coord.y > -line * 0.1 && coord.y < line * 0.1 )
    {
        outColor.x = color;
        outColor.y = 0.0f;
        outColor.z = 0.0f;
    }
    if (coord.x > -line * 0.1f && coord.x < line * 0.1f )
    {
        outColor.x = 0.0f;
        outColor.y = color;
        outColor.z = 0.0f;
    }

    // Faloff to make the grid fade when it's away from the camera. 
    // I've added the rand factor to the falloff smoothstep to add some noise and minimize color banding.
    float distanceToCamera = length(fragCoords - ubo.cameraPos.xy);
    float sqDist = 2.0 * 2.0 * dot(coord * 0.5f, coord * 0.5f);
    opacityFalloff = smoothstep(1.5f, 0.0f, distanceToCamera / maxFadeDistance) * clamp(rand(fragCoords.xy), 0.7f, 1.0f);

    outColor.w *= opacityFalloff;
}