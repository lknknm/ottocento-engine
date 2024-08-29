#version 450

vec2 gridPlane[6] = vec2[]( vec2(-150.0f, -150.0f),
                            vec2(150.0f, -150.0f),
                            vec2(150.0f, 150.0f),
                            vec2(-150.0f, -150.0f),
                            vec2(-150.0f, 150.0f),
                            vec2(150.0f, 150.0f)
                            );

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
} ubo;

layout(location = 0) out vec2 fragCoords;

void main() {
    fragCoords  = gridPlane[gl_VertexIndex] + ubo.cameraPos.xy;
    gl_Position = ubo.proj * ubo.view * vec4(gridPlane[gl_VertexIndex] + ubo.cameraPos.xy, 0, 1.0);
}