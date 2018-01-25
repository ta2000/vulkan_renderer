#ifndef GAME_H_
#define GAME_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>
#include <stdint.h>

struct mouse
{
    float x, y;
    float dx, dy;
};

struct game
{
    bool running;
    struct mouse mouse;
    bool keys[GLFW_KEY_LAST];
    bool keys_prev[GLFW_KEY_LAST];
    struct renderer_resources* renderer_resources;
};

void game_run(struct game* game);
void game_process_input(struct game* game);
void game_update(struct game* game);
void game_render(struct game* game);

void game_resize(
    struct game* game,
    int width,
    int height
);

void game_update_keys(
    struct game* game,
    int key_index,
    bool pressed
);

void game_update_mouse_pos(
    struct game* game,
    double x,
    double y
);

#endif
