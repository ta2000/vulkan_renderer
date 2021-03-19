/*#ifndef TABLE_H_
#define TABLE_H_

#include <stdbool.h>

struct table_entry
{
    char *key;
    size_t key_size;
    char *value;
    size_t value_size;
};

struct table
{
    struct draw_table_entry *entries;
    int entry_count;
    bool (*cmp_func)(char *, char *);
};

void table_init(
    struct table *table
);

char *table_search(
    struct draw_table *draw_table,
    struct renderer_draw_command* draw_command
);

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
    struct draw_table *draw_table
);

#endif
*/