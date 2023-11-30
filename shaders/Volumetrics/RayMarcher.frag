#version 450

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInputMS inputDepth;

void main() {
    float depth = subpassLoad(inputDepth, 8).r;
    outColor = vec4(0, 0.5, depth, 0.5);
}