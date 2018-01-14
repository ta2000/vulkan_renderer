#ifndef RENDERER_TOOLS_H_
#define RENDERER_TOOLS_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

size_t renderer_get_file_size(
    const char* fname
);

void renderer_read_file_to_buffer(
    const char* fname,
    char** buffer,
    size_t buffer_size
);

uint32_t renderer_find_memory_type(
	uint32_t memory_type_bits,
	VkMemoryPropertyFlags properties,
	uint32_t memory_type_count,
	VkMemoryType* memory_types
);

#endif
