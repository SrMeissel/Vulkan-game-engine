#version 450

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;


void main() {
    outColor = vec4(0, 0, 1, gl_FragDepth/1);
}