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
    vec4 position;
    vec4 color;
    float intensity; // 5    
    float radius; // 2
    // intensity can be color.w, radius can be position.w just saying
} push;



void main() {
    vec4 Color = subpassLoad(inColor);
    vec4 Position = subpassLoad(inPosition);
    vec4 Normal = normalize(subpassLoad(inNormal));

    vec3 diffuseLight = vec3(0.001);
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - Position.xyz);

    vec3 lightDirection = push.position.xyz - Position.xyz;
    //float attenuation = 1.0 / (dot(lightDirection, lightDirection) * 5.0);
    
    float attenuation = (2/(push.radius*push.radius))*(1 - (sqrt(dot(lightDirection, lightDirection))/sqrt(dot(lightDirection, lightDirection)+(push.radius*push.radius))));
 
    lightDirection = normalize(lightDirection);

    float cosangIncidence = max(dot(Normal.xyz, lightDirection), 0);
    vec3 intensity = push.color.xyz * attenuation * push.intensity;

    diffuseLight += intensity * cosangIncidence;

    //specular Lighting
    vec3 halfAngle = normalize(lightDirection + viewDirection);
    float blinnTerm = max(dot(Normal.xyz, halfAngle), 0.0);
    blinnTerm = pow(blinnTerm, 8.0); //higher values = sharper light
    specularLight += intensity * blinnTerm;

    vec4 THELIGHT = vec4(diffuseLight + specularLight, 1.0);
    outColor = THELIGHT * Color;
    //vec4(specularLight, 1.0);
    //outColor = subpassLoad(inNormal);
}