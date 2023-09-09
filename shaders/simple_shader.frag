#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;
//layout(location = 0) in vec3 fragColor;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientColor;
    PointLight pointLights[10];
    int numLights;
} ubo;


layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    vec3 diffuseLight = ubo.ambientColor.xyz * ubo.ambientColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for(int i=0; i<ubo.numLights; i++){
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight); // dot of the same matrix gives the length squared, i guess
        float cosangIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosangIncidence;
    }
    outColor = vec4(diffuseLight * fragColor, 1.0);
}