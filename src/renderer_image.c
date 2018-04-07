#include "renderer_buffer.h"
#include "renderer_tools.h"
#include "renderer_image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct renderer_image renderer_get_image(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkExtent2D extent,
        VkFormat format,
        VkImageAspectFlags aspect_mask,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memory_flags)
{
    struct renderer_image image;
    memset(&image, 0, sizeof(image));

    image.width = extent.width;
    image.height = extent.height;

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {extent.width, extent.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
    };

    VkResult result;
    result = vkCreateImage(device, &image_info, NULL, &image.image);
    assert(result == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(device, image.image, &mem_reqs);

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = renderer_find_memory_type(
            mem_reqs.memoryTypeBits,
            memory_flags,
            mem_props.memoryTypeCount,
            mem_props.memoryTypes
        )
    };

    result = vkAllocateMemory(device, &alloc_info, NULL, &image.memory);
    assert(result == VK_SUCCESS);

    vkBindImageMemory(device, image.image, image.memory, 0);

    VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange.aspectMask = aspect_mask,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1
    };
    result = vkCreateImageView(
        device,
        &image_view_info,
        NULL,
        &image.image_view
    );
    assert(result == VK_SUCCESS);

    return image;
}

struct renderer_image renderer_get_sampled_image(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkExtent2D extent,
        VkFormat format,
        VkImageAspectFlags aspect_mask,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memory_flags)
{
    struct renderer_image image;
    image = renderer_get_image(
        physical_device,
        device,
        extent,
        format,
        aspect_mask,
        tiling,
        usage,
        memory_flags
    );

    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias =0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkResult result;
    result = vkCreateSampler(
        device,
        &sampler_info,
        NULL,
        &image.sampler
    );
    assert(result == VK_SUCCESS);

    return image;
}

void renderer_change_image_layout(
        VkDevice device,
        VkQueue queue,
        VkCommandPool command_pool,
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkAccessFlagBits src_access_mask,
        VkImageAspectFlags aspect_mask)
{
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkResult result;
    result = vkAllocateCommandBuffers(
        device,
        &cmd_alloc_info,
        &cmd
    );
    assert(result == VK_SUCCESS);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    result = vkBeginCommandBuffer(
        cmd,
        &cmd_begin_info
    );
    assert(result == VK_SUCCESS);

    VkImageMemoryBarrier memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = src_access_mask,
        .dstAccessMask = 0,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            aspect_mask,
            0,
            1,
            0,
            1
        }
    };

    VkPipelineStageFlags src_stage = 0;
    VkPipelineStageFlags dst_stage = 0;

    if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    /*if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }*/

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        memory_barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        memory_barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT |
            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        cmd,
        src_stage,
        dst_stage,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &memory_barrier
    );

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL
    };

    result = vkQueueSubmit(
        queue,
        1,
        &submit_info,
        VK_NULL_HANDLE
    );
    assert(result == VK_SUCCESS);

    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(
        device,
        command_pool,
        1,
        &cmd
    );
}

struct renderer_image renderer_load_texture(
    const char* src,
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkQueue queue,
    VkCommandPool command_pool)
{
    struct renderer_image tex_image;

    stbi_uc* pixels = NULL;
    int tex_width, tex_height, tex_channels;
    pixels = stbi_load(
        src,
        &tex_width,
        &tex_height,
        &tex_channels,
        STBI_rgb_alpha
    );
    assert(pixels && tex_width && tex_height);

    VkDeviceSize image_size = tex_width * tex_height * 4;

    VkExtent2D extent = {.width = tex_width, .height = tex_height};
    tex_image = renderer_get_sampled_image(
        physical_device,
        device,
        extent,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    struct renderer_buffer staging_buffer;
    staging_buffer = renderer_get_buffer(
        physical_device,
        device,
        image_size,
        0,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    void* mapped;
    VkResult result;
    result = vkMapMemory(
        device,
        staging_buffer.memory,
        0,
        image_size,
        0,
        &mapped
    );
    assert(result == VK_SUCCESS);
    memcpy(mapped, pixels, (size_t)image_size);
    vkUnmapMemory(device, staging_buffer.memory);

    stbi_image_free(pixels);

    renderer_change_image_layout(
        device,
        queue,
        command_pool,
        tex_image.image,
        VK_IMAGE_LAYOUT_PREINITIALIZED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkExtent3D copy_extent = {tex_width, tex_height, 1};
    renderer_copy_buffer_to_image(
        device,
        command_pool,
        queue,
        staging_buffer.buffer,
        tex_image.image,
        copy_extent
    );

    renderer_change_image_layout(
        device,
        queue,
        command_pool,
        tex_image.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    vkDestroyBuffer(device, staging_buffer.buffer, NULL);
    vkFreeMemory(device, staging_buffer.memory, NULL);

    return tex_image;
}
