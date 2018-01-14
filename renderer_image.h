#ifndef RENDERER_IMAGE_H_
#define RENDERER_IMAGE_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct renderer_image
{
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkSampler sampler;
    uint32_t width, height;
};

struct renderer_image renderer_get_image(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkExtent3D extent,
    VkFormat format,
    VkImageAspectFlags aspect_mask,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags
);

void renderer_change_image_layout(
    VkDevice device,
    VkQueue queue,
    VkCommandPool command_pool,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlagBits src_access_mask,
    VkImageAspectFlags aspect_mask
);

struct renderer_image renderer_load_texture(
    const char* src,
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkQueue queue,
    VkCommandPool command_pool
);

#endif
