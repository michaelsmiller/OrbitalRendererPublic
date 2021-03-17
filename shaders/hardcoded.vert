#version 450
#extension GL_ARB_separate_shader_objects : enable

// binding is like location for uniforms
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camera_pos;
    vec3 light_pos;
    vec3 light_color;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in uint inRenderType;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) flat out uint fragRenderType;

// gl_VertexIndex is an implicit input variable for vertex shaders
// gl_Position is a built_in output
void main() 
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragNormal = inNormal;
    fragPos = inPosition;
    fragRenderType = inRenderType;
}
