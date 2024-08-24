#version 450

vec2 gridPlane[6] = vec2[](vec2(-50.0f, -50.0f),
                            vec2(50.0f, -50.0f),
                            vec2(50.0f, 50.0f),
                            vec2(-50.0f, -50.0f),
                            vec2(-50.0f, 50.0f),
                            vec2(50.0f, 50.0f)
                            );

layout(location = 0) out vec2 fragCoords;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(gridPlane[gl_VertexIndex], 0, 1.0);
    fragCoords  = gridPlane[gl_VertexIndex];
}