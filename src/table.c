#include "table.h"

#include <stdlib.h>

void table_init(
    struct table *table,
    size_t key_size,
    size_t value_size,
    int max_entry_count)
{
    table->entry_count = max_entry_count;
    table->entries = malloc(max_entry_count * sizeof(*table->entries));

    for (int i = 0; i < max_entry_count; i++) {
        table->entries[i].key = malloc(key_size);
        table->entries[i].key_size = key_size;
        table->entries[i].value = malloc(value_size);
        table->entries[i].value_size = value_size;
    }
}

/*
char *table_search(
    struct table *table,
    void *key)
{
    for (int i = 0; i < table->entry_count; i++) {
        if (
    }
}
*/

void table_insert(
    struct draw_table *draw_table,
    struct renderer_draw_command *draw_command,
    struct renderer_drawable *drawable
);

void table_delete(
    struct draw_table *draw_table,
    struct renderer_draw_command *draw_command
);

void table_destroy(
    struct table *table)
{
    for (int i = 0; i < table->entry_count; i++) {
        free(table->entries[i].key);
        free(table->entries[i].value)
    }

    free(table->entries);
}
