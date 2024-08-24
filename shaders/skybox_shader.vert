#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec2 uv;

layout(location = 0) out vec3 forwards;

out gl_PerVertex {
	vec4 gl_Position;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
} ubo;

layout(push_constant) uniform Push {
	mat4 rotation;
} push;

void main() {
	vec4 position = vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0f - 1.0f, 0.0f, 1.0f);

	mat4 inverseProjection = inverse(ubo.projection);
	vec3 unprojected = (inverseProjection * position).xyz;
	forwards = mat3(ubo.inverseView) * unprojected;

	gl_Position = position;
}