#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec2 uv;

layout(location = 0) out vec4 fragPos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    vec4 worldPosition = push.modelMatrix * vec4(position, 1.0);
    fragPos = ubo.projection * ubo.view * worldPosition;
    gl_Position = fragPos;
}