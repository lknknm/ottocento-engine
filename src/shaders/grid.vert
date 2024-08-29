#version 450

vec2 gridPlane[6] = vec2[]( vec2(-50.0f, -50.0f),
                            vec2(50.0f, -50.0f),
                            vec2(50.0f, 50.0f),
                            vec2(-50.0f, -50.0f),
                            vec2(-50.0f, 50.0f),
                            vec2(50.0f, 50.0f)
                            );

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
} ubo;

const float gridSize = 100.0f;
layout(location = 0) out vec2 fragCoords;

float planeSDF(vec3 p) {
    // Define the plane parameters (normal and distance to origin)
    vec3 normal = vec3(0.0, 1.0, 0.0); // Assuming Z is up
    float distance = 0.0; // Distance of the plane from origin

    return dot(p, normal) + distance;
}

void main() {
    float dist = planeSDF(ubo.cameraPos);
    vec3 offset = normalize(ubo.cameraPos) * dist;
    gl_Position = ubo.proj * ubo.view * vec4(gridPlane[gl_VertexIndex], 0, 1.0);
    fragCoords  = gridPlane[gl_VertexIndex];
}