#version 450

layout (location = 0) in vec4 fragPos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    float near;
    float far;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    gl_FragDepth = ( fragPos.z - ubo.near ) / ( ubo.far - ubo.near );
}