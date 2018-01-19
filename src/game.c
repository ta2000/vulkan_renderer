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

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    struct game* game;
    game = (struct game*)glfwGetWindowUserPointer(window);

    game->mouse.dx = game->mouse.x - xpos;
    game->mouse.dy = game->mouse.y - ypos;
    game->mouse.x = xpos;
    game->mouse.y = ypos;
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
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    renderer_initialize_resources(game->renderer_resources, window);

    float tmp_yaw = 4.0f;
    float tmp_pitch = 2.0f;
    float sensitivity = 0.01f;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (game->running) {
            tmp_yaw += game->mouse.dx * sensitivity;
            tmp_pitch += game->mouse.dy * sensitivity;
            renderer_update_camera(
                &game->renderer_resources->camera,
                4.0f, 4.0f, 2.0f,
                tmp_pitch, tmp_yaw
            );

            game_update(game);
            game_render(game);
        }

        game->mouse.dx = 0.f;
        game->mouse.dy = 0.f;
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
