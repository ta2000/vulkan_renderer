#include "renderer_tools.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

size_t renderer_get_file_size(
        const char* fname)
{
    FILE* fp = fopen(fname, "r");
    assert(fp);

    size_t fsize;
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);

    fclose(fp);

    return fsize;
}

void renderer_read_file_to_buffer(
        const char* fname,
        char** buffer,
        size_t buffer_size)
{
    FILE* fp = fopen(fname, "r");
    assert(fp);

    assert(buffer);

    size_t bytes_read;
    bytes_read = fread(*buffer, 1, buffer_size, fp);
    assert(bytes_read == buffer_size);

    fclose(fp);
}

uint32_t renderer_find_memory_type(
        uint32_t memory_type_bits,
        VkMemoryPropertyFlags properties,
        uint32_t memory_type_count,
        VkMemoryType* memory_types)
{
    uint32_t memory_type;

    uint32_t i;
    bool memory_type_found = false;
    for (i=0; i<memory_type_count; ++i)
    {
        if ((memory_type_bits & (1 << i)) &&
            (memory_types[i].propertyFlags & properties) == properties)
        {
            memory_type = i;
            memory_type_found = true;
            break;
        }
    }

    assert(memory_type_found);

    return memory_type;
}
