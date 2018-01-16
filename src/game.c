#include "renderer_buffer.h"
#include "renderer_image.h"
#include "renderer_tools.h"
#include "renderer.h"
#include "game.h"

#include <stdlib.h>
#include <string.h>

void game_run(struct game* game)
{
    memset(game, 0, sizeof(*game));

    game->renderer_resources = malloc(sizeof(*game->renderer_resources));

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Vulkan Window",
        NULL,
        NULL
    );
    glfwSetWindowUserPointer(window, game->renderer_resources);
    glfwSetWindowSizeCallback(window, renderer_resize);

    renderer_initialize_resources(game->renderer_resources, window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        game_update(game);
        game_render(game);
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
