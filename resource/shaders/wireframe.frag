#version 450
#extension GL_EXT_buffer_reference : require

layout(binding = 1) uniform sampler2D texSampler[1024];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstantData {
    vec3 offset;
    vec3 color;
    uint textureID;
} push;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    outColor = vec4(vec3(0.2, 0.2, 0.2) * clamp(rand(fragTexCoord), 0.5f, 1.0f), 1.0);
}