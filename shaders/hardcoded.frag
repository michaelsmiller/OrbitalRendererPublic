#version 450
#extension GL_ARB_separate_shader_objects : enable

// location goes from 0 to 2 because the position is a double array and they
// use 32 bit as the standard "slot" size I think

/* layout(location = 0) in dvec3 inPosition; */
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
