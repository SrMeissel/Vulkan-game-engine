#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;
layout (location = 4) in mat3 TBN;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPosition;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
} ubo;

layout(set = 1, binding = 0) uniform sampler Sampler;

layout(set = 1, binding = 1) uniform texture2D albedoValue;
layout(set = 1, binding = 2) uniform texture2D normalValue;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    vec3 VkNormal = (texture(sampler2D(normalValue, Sampler), fragUv).xyz);
    // VkNormal.x = -VkNormal.x;
    VkNormal.y = -VkNormal.y;
    
    vec3 Normal = normalize(TBN * VkNormal * 2.0 - 1.0);
    // Normal.x = -Normal.x;
    //Normal.y = -Normal.y;

    outNormal = vec4(Normal, 1.0);

    outPosition = vec4(fragPosWorld, 1.0);

    outColor = texture(sampler2D(albedoValue, Sampler), fragUv);
}