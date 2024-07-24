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

layout(push_constant) uniform Push {
    vec3 position;
    vec3 color;
} push;


void main() {
    vec3 diffuseLight = vec3(0.01);
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - subpassLoad(inPosition).xyz);

    vec3 lightDirection = push.position - subpassLoad(inPosition).xyz;
    float attenuation = 1.0f / dot(lightDirection, lightDirection); // dot of the same matrix gives the length squared, i guess
    
    lightDirection = normalize(lightDirection);

    float cosangIncidence = max(dot(subpassLoad(inNormal).xyz, lightDirection), 0);
    vec3 intensity = push.color.xyz * attenuation * 50.0f;

    diffuseLight += intensity * cosangIncidence;

    //specular Lighting
    vec3 halfAngle = normalize(lightDirection + viewDirection);
    float blinnTerm = dot(subpassLoad(inNormal).xyz, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 32.0); //higher values = sharper light
    specularLight += intensity * blinnTerm;

    outColor = vec4(diffuseLight * subpassLoad(inColor).xyz + specularLight * subpassLoad(inColor).xyz, 1.0);
    //outColor = vec4(1.0, 1.0, 1.0, 1.0) * (subpassLoad(inPosition) * subpassLoad(inNormal) * subpassLoad(inColor));
}