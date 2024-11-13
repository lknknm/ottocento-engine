#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 1) uniform sampler2D texSampler[1000];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;
layout(location = 3) in float lightIntensity;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstantData {
    vec3 offset;
    vec3 color;
    uint textureID;
} push;

void main() {
    if (texture(texSampler[push.textureID], fragTexCoord).a == 0.0)
        outColor = vec4(fragColor, 1.0f);
    else
    {
        vec4 texColor = texture(texSampler[push.textureID], fragTexCoord);
        vec3 modifiedColor = texColor.rgb * lightIntensity;
        vec4 finalColor = vec4(modifiedColor, texColor.a);
        outColor = finalColor;
    }
}