#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference : require

vec2 gridPlane[6] = vec2[]( vec2(-500.0f, -500.0f),
                            vec2(500.0f, -500.0f),
                            vec2(500.0f, 500.0f),
                            vec2(-500.0f, -500.0f),
                            vec2(-500.0f, 500.0f),
                            vec2(500.0f, 500.0f)
                            );

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 normalMatrix;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
    uint64_t edgeBuffer;
} ubo;

layout(location = 0) out vec2 fragCoords;

void main() {
    fragCoords  = gridPlane[gl_VertexIndex] + ubo.cameraPos.xy;
    gl_Position = ubo.proj * ubo.view * vec4(gridPlane[gl_VertexIndex] + ubo.cameraPos.xy, 0, 1.0);
}