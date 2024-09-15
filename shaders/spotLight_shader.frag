#version 450
layout (location = 0) out vec4 outColor;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inColor;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inNormal;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inPosition;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
} ubo;

layout(set = 2, binding = 0) uniform sampler Sampler;
layout(set = 2, binding = 1) uniform texture2D shadowMap;

layout(push_constant) uniform Push {
    mat4 lightMatrix;
    vec4 position;
    vec4 color;
    float intensity;  
} push;

void main() {
    // Transform fragment position to light space
    vec4 fragPosLightSpace = push.lightMatrix * vec4(subpassLoad(inPosition).xyz, 1.0);

    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range

    // Sample shadow map
    float closestDepth = texture(sampler2D(shadowMap, Sampler), projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Shadow factor
    float shadow = currentDepth > closestDepth + 0.005 ? 1.0 : 0.0; // Add bias to avoid shadow acne

    outColor = vec4(shadow);
}