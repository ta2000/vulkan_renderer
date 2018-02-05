#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UboInstance {
    mat4 model;
} uboInstance;

layout(binding = 1) uniform UboView {
    mat4 view;
    mat4 proj;
} uboView;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 modelView = uboView.view * uboInstance.model;
    gl_Position = uboView.proj * modelView * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
