#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding is like location for uniforms
layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition; // from vertex buffer
layout(location = 1) in vec3 inColor; // from vertex buffer

layout(location = 0) out vec3 fragColor; // output to fragment shader

// gl_VertexIndex is an implicit input variable for vertex shaders
// gl_Position is a built_in output
void main() {
  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
  fragColor = inColor;
}
