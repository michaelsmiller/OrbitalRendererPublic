#version 450
#extension GL_ARB_separate_shader_objects : enable

// location goes from 0 to 2 because the position is a double array and they
// use 32 bit as the standard "slot" size I think

/* layout(location = 0) in dvec3 inPosition; */

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camera_pos;
    vec3 light_pos;
    vec3 light_color;
} ubo;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) flat in uint fragRenderType;

layout(location = 0) out vec4 outColor;

vec3 Phong_BRDF(vec3 L, vec3 V, vec3 N, vec3 diffuse_color, vec3 specular_color, float specular_exponent)
{
    vec3 R = 2 * dot(L, N) * N - L;
    vec3 phong_color = specular_color * pow(max(dot(R, V), 0), specular_exponent);
    return phong_color;
}

void main()
{
    switch (fragRenderType)
    {
    case 0: // molecule
        vec3 N = normalize(fragNormal);
        vec3 L = normalize(ubo.light_pos - fragPos);
        vec3 V = normalize(ubo.camera_pos - fragPos);

        vec3 color = 0.2 * fragColor; // ambient
        color += ubo.light_color * Phong_BRDF(L, V, N, fragColor, vec3(1,1,1), 3);
        // No decay of light over distance is accounted.

        outColor = vec4(color, 1);
        break;
    case 1: // orbital
        outColor = vec4(fragColor, 0.25);
        break;
    default:
        outColor = vec4(0.5,0.5,0.5,1);
        break;
    }
}
