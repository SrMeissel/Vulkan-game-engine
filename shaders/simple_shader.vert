#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientColor;
    vec3 lightPostition;
    vec4 lightColor;
} ubo;

// layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
    vec4 worldPosition = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionViewMatrix * worldPosition;

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    vec3 directionToLight = ubo.lightPostition - worldPosition.xyz;
    float attenuation = 1.0f / dot(directionToLight, directionToLight); // dot of the same matrix gives the length squared, i guess

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
    vec3 ambientLight = ubo.ambientColor.xyz * ubo.ambientColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

    fragColor = (diffuseLight + ambientLight) * color;
}