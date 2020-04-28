#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferViewProjection {
    mat4 view_projection;
} ubo_vp;

layout(binding = 1) uniform UniformBufferModel {
    mat4 model;
} ubo_m;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo_vp.view_projection * ubo_m.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
