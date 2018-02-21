#ifndef RENDERER_MESH_H_
#define RENDERER_MESH_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdint.h>

struct renderer_mesh
{
    struct renderer_buffer* vbo;
    uint32_t vbo_offset;
    struct renderer_buffer* ibo;
    uint32_t ibo_offset;
    uint32_t index_count;
    struct renderer_buffer* uniform_buffer;
    uint32_t uniform_offset;
    VkCommandBuffer cmd;
    VkPipeline pipeline;
};

void bind(struct renderer_mesh* mesh);

#endif
