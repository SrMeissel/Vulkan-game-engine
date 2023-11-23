#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout(location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;
//layout(location = 0) in vec3 fragColor;

struct PointLight {
  vec4 position;
  vec4 color;
};


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

//trying to make this 'set = 1'
layout(set = 0, binding = 1) uniform sampler texSampler;

layout(set = 0, binding = 2) uniform texture2D textures[3];


layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
    int textureIndex;
} push;

void main() {
    vec3 diffuseLight = ubo.ambientColor.xyz * ubo.ambientColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    //per light object
    for(int i=0; i<ubo.numLights; i++){
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(directionToLight, directionToLight); // dot of the same matrix gives the length squared, i guess

        directionToLight = normalize(directionToLight);


        float cosangIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosangIncidence;

        //specular Lighting
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 32.0); //higher values = sharper light
        specularLight += intensity * blinnTerm;
    }

    vec4 fragLight = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
    outColor = texture(sampler2D(textures[push.textureIndex], texSampler), fragTexCoord) * fragLight;
}