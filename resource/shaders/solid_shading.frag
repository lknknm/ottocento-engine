#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 normalMatrix;
    mat4 view;
    mat4 proj;
    mat4 inverseproj;
    vec3 cameraPos;
} ubo;
layout(binding = 1) uniform sampler2D texSampler[1024];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in float lightIntensity;

layout(location = 4) in vec3 normal;
layout(location = 5) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstantData {
    vec3 offset;
    vec3 color;
    uint textureID;
} push;

void main() {
    vec3 E = normalize(viewPos);
    vec3 N = normalize(normal);
    vec3 reflected = reflect(E, N);

    float m = sqrt(
        pow(reflected.x, 2.0) +
        pow(reflected.y, 2.0) +
        pow(reflected.z + 1.0, 2.0)
    );
    vec2 uv = (reflected.xy / -m) * 0.5 + 0.5;
    
    vec4 matcapColor = texture(texSampler[2], uv);
    outColor = vec4(matcapColor.rgb, 1.0f);
}
