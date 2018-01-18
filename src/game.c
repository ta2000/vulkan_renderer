#include "renderer_buffer.h"
#include "renderer_image.h"
#include "renderer_tools.h"
#include "renderer.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void resize_callback(GLFWwindow* window, int width, int height)
{
    struct game* game;
    game = (struct game*)glfwGetWindowUserPointer(window);

    renderer_resize(game->renderer_resources, width, height);
}

static void key_callback(
        GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        int mode = glfwGetInputMode(window, GLFW_CURSOR);

        if (mode == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void game_run(struct game* game)
{
    memset(game, 0, sizeof(*game));
    game->running = true;

    game->renderer_resources = malloc(sizeof(*game->renderer_resources));

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Vulkan Window",
        NULL,
        NULL
    );
    glfwSetWindowUserPointer(window, game);
    glfwSetWindowSizeCallback(window, resize_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    renderer_initialize_resources(game->renderer_resources, window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (game->running) {
            game_update(game);
            game_render(game);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    renderer_destroy_resources(game->renderer_resources);
    free(game->renderer_resources);
}

void game_update(struct game* game)
{
    // Update code
}

void game_render(struct game* game)
{
    drawFrame(game->renderer_resources);
}
