#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference : require

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 normalMatrix;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
    uint64_t edgesBuffer;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out float lightIntensity;

layout(location = 4) out vec3 normal;
layout(location = 5) out vec3 viewPos;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -1.0, 5.0));
const float AMBIENT = 0.3;

layout(push_constant) uniform PushConstantData {
    vec3 offset;
    vec3 color;
} push;

void main() {
    normal  = (ubo.view * ubo.model * vec4(inNormal, 0.0)).xyz;
    viewPos = (ubo.view * ubo.model * vec4(inPosition, 1.0)).xyz;
    
    vec3 normalWorldSpace = normalize((ubo.normalMatrix * vec4(inNormal, 0.0f)).xyz);
    lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);
    
    glGenBuffers(1, &ubo.edgesBuffer);
    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
            
    fragColor    = inColor;
    fragTexCoord = inTexCoord;
    fragPosition = inPosition;
}