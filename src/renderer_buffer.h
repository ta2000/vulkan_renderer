#ifndef RENDERER_BUFFER_H_
#define RENDERER_BUFFER_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct renderer_buffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
    void* mapped;
};

struct renderer_buffer renderer_get_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memory_flags
);

void renderer_destroy_buffer(
    VkDevice device,
    struct renderer_buffer* buffer
);

void renderer_map_buffer(
    VkDevice device,
    VkDeviceSize offset,
    struct renderer_buffer* buffer
);

void renderer_unmap_buffer(
    VkDevice device,
    struct renderer_buffer* buffer
);

void renderer_copy_buffer_to_buffer(
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    VkBuffer src_buffer,
    VkBuffer dst_buffer,
    VkDeviceSize mem_size
);

void renderer_copy_buffer_to_image(
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    VkBuffer src_buffer,
    VkImage dst_image,
    VkExtent3D extent
);

#endif
