#version 450

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    float nearPlane;
    float farPlane;
} ubo;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inColor;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inNormal;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inPosition;

layout(set = 2, binding = 0) uniform sampler Sampler;
layout(set = 2, binding = 1) uniform texture2D shadowMap;

layout(push_constant) uniform Push {
    mat4 lightMatrix;
    vec4 position;
    vec4 color;
    float intensity;  
} push;


void main() {
    // Light =====================================================================================================

    vec4 Color = subpassLoad(inColor);
    vec4 Position = subpassLoad(inPosition);
    vec4 Normal = normalize(subpassLoad(inNormal));

    vec3 diffuseLight = vec3(0.001);
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - Position.xyz);

    vec3 lightDirection = push.position.xyz - Position.xyz;
    float attenuation = 1.0 / (dot(lightDirection, lightDirection) * 5.0);
 
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

    // Shadow ====================================================================================================

    // Transform fragment position to light space
    vec4 fragPosLightSpace = push.lightMatrix * vec4(subpassLoad(inPosition).xyz, 1.0);

    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Sample shadow map
    float closestDepth = texture(sampler2D(shadowMap, Sampler), projCoords.xy).r;
    float currentDepth = ( fragPosLightSpace.z - ubo.nearPlane ) / ( ubo.farPlane - ubo.nearPlane );

    // Shadow factor
    //float bias = max(0.05 * (1.0 - dot(Normal.xyz, lightDirection.xyz)), 0.005);  
    //float bias = 0.005;
    float bias = 0.0001;
    float shadow = currentDepth > closestDepth + bias ? 1.0 : 0.0;
    if(projCoords.z >= 1.0) shadow = 1.0;

    outColor = THELIGHT * Color * vec4(max(1-shadow, 0));
    //outColor = vec4(vec3(shadow), 1.0); 
}