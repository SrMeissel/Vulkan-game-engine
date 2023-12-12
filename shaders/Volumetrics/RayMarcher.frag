#version 450
#define VULKAN 100

//#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable

//layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 outColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputDepth; //subpassInputMS 

void main() {
    float depth = subpassLoad(inputDepth).r; // (inputDepth, 8)
    //outColor = vec4(0, 0.05, depth*10, 0.5);

    outColor = vec4(0, 0, 1, depth);


}