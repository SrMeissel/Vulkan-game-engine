#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;
//layout(location = 0) in vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientColor;
    vec3 lightPostition;
    vec4 lightColor;
} ubo;


layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    vec3 directionToLight = ubo.lightPostition - fragPosWorld.xyz;
    float attenuation = 1.0f / dot(directionToLight, directionToLight); // dot of the same matrix gives the length squared, i guess

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientColor.xyz * ubo.ambientColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}