#include "renderer_tools.h"
#include "renderer_buffer.h"

#include <assert.h>

struct renderer_buffer renderer_get_buffer(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memory_flags)
{
    struct renderer_buffer buffer;
    buffer.size = size;

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };
    vkCreateBuffer(device, &buffer_info, NULL, &buffer.buffer);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &mem_reqs);

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

    VkResult result;
    result = vkAllocateMemory(
        device,
        &alloc_info,
        NULL,
        &buffer.memory
    );
    assert(result == VK_SUCCESS);

    vkBindBufferMemory(
        device,
        buffer.buffer,
        buffer.memory,
        0
    );

    return buffer;
}

void renderer_copy_buffer_to_buffer(
        VkDevice device,
        VkCommandPool command_pool,
        VkQueue queue,
        VkBuffer src_buffer,
        VkBuffer dst_buffer,
        VkDeviceSize mem_size)
{
    VkCommandBuffer copy_cmd;
    VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkResult result;
    result = vkAllocateCommandBuffers(device, &cmd_alloc_info, &copy_cmd);
    assert(result == VK_SUCCESS);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    result = vkBeginCommandBuffer(copy_cmd, &cmd_begin_info);
    assert(result == VK_SUCCESS);

    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = mem_size
    };

    vkCmdCopyBuffer(
        copy_cmd,
        src_buffer,
        dst_buffer,
        1,
        &region
    );

    vkEndCommandBuffer(copy_cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &copy_cmd,
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
        &copy_cmd
    );
}

void renderer_copy_buffer_to_image(
        VkDevice device,
        VkCommandPool command_pool,
        VkQueue queue,
        VkBuffer src_buffer,
        VkImage dst_image,
        VkExtent3D extent)
{
    VkCommandBuffer copy_cmd;
    VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkResult result;
    result = vkAllocateCommandBuffers(device, &cmd_alloc_info, &copy_cmd);
    assert(result == VK_SUCCESS);

    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    result = vkBeginCommandBuffer(copy_cmd, &cmd_begin_info);
    assert(result == VK_SUCCESS);

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = extent
    };

    vkCmdCopyBufferToImage(
        copy_cmd,
        src_buffer,
        dst_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    vkEndCommandBuffer(copy_cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &copy_cmd,
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
        &copy_cmd
    );
}
