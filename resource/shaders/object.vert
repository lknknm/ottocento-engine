#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out float lightIntensity;

layout(push_constant) uniform PushConstantData {
    vec3 offset;
    vec3 color;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -1.0, 5.0));
const float AMBIENT = 0.02;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition + push.offset, 1.0);
    vec3 normalWorldSpace = normalize((ubo.model * vec4(normal, 0.0f)).xyz);
    lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);

    fragColor    = lightIntensity * inColor;
    fragTexCoord = inTexCoord;
    fragPosition = inPosition;
}