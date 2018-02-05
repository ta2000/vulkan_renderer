#version 450
#extension GL_ARB_seperate_shader_objects : enable

layout(binding = 2) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(tex_sampler, fragTexCoord);
}
