#version 450 

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

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

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

void main() {
    const float M_PI = 3.1415926538;
    float dis =sqrt(dot(fragOffset, fragOffset));
    if(dis >= 1) {
        discard;
    }

    outColor = vec4(push.color.xyz, 0.5 * (cos(dis * M_PI) +1.0));
}