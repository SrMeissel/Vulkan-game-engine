#version 450

layout(location = 0) in vec3 forwards;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube skybox;

layout(push_constant) uniform Push {
	mat4 rotation;
} push;


//https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
void main() {
    vec3 direction = (vec4(forwards, 0.0) * push.rotation).xyz;
    vec4 sampledColor = texture(skybox, direction);
    outColor = sampledColor;
    gl_FragDepth = 1.0;
}