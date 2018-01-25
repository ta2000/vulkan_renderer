#include "renderer_buffer.h"
#include "renderer_image.h"
#include "renderer_tools.h"
#include "renderer.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void resize_callback(GLFWwindow* window, int width, int height)
{
    struct game* game;
    game = (struct game*)glfwGetWindowUserPointer(window);

    game_resize(game, width, height);
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

    struct game* game;
    game = (struct game*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS)
        game_update_keys(game, key, true);
    if (action == GLFW_RELEASE)
        game_update_keys(game, key, false);
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    struct game* game;
    game = (struct game*)glfwGetWindowUserPointer(window);

    game_update_mouse_pos(game, xpos, ypos);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    renderer_initialize_resources(game->renderer_resources, window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        game_process_input(game);
        if (game->running) {
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

void game_process_input(struct game* game)
{
    struct camera* camera = &game->renderer_resources->camera;

    if (game->running) {
        float cam_speed = 0.0035f;
        float sensitivity = 0.005f;

        if (game->keys[GLFW_KEY_W]) {
            camera->x += cosf(camera->yaw) * cam_speed;
            camera->y += sinf(camera->yaw) * cam_speed;
        }
        if (game->keys[GLFW_KEY_S]) {
            camera->x -= cosf(camera->yaw) * cam_speed;
            camera->y -= sinf(camera->yaw) * cam_speed;
        }
        if (game->keys[GLFW_KEY_A]) {
            camera->x += cosf(camera->yaw + M_PI/2) * cam_speed;
            camera->y += sinf(camera->yaw + M_PI/2) * cam_speed;
        }
        if (game->keys[GLFW_KEY_D]) {
            camera->x -= cosf(camera->yaw + M_PI/2) * cam_speed;
            camera->y -= sinf(camera->yaw + M_PI/2) * cam_speed;
        }

        if (game->keys[GLFW_KEY_LEFT_SHIFT]) {
            camera->z += cam_speed;
            camera->pitch += cam_speed;
        }
        if (game->keys[GLFW_KEY_LEFT_CONTROL]) {
            camera->z -= cam_speed;
            camera->pitch -= cam_speed;
        }

        camera->yaw += game->mouse.dx * sensitivity;
        camera->pitch += game->mouse.dy * sensitivity;
    }

    if (game->keys[GLFW_KEY_ESCAPE] && !game->keys_prev[GLFW_KEY_ESCAPE])
    {
        game->running = !game->running;
    }

    memcpy(game->keys_prev, game->keys, GLFW_KEY_LAST * sizeof(game->keys[0]));
}

void game_update(struct game* game)
{
    // Update code
}

void game_render(struct game* game)
{
    drawFrame(game->renderer_resources);
}

void game_resize(
        struct game* game,
        int width, int height)
{
    renderer_resize(game->renderer_resources, width, height);
}

void game_update_keys(
        struct game* game,
        int key_index,
        bool pressed)
{
    game->keys[key_index] = pressed;
}

void game_update_mouse_pos(
        struct game* game,
        double x,
        double y)
{
    game->mouse.dx = game->mouse.x - (float)x;
    game->mouse.dy = game->mouse.y - (float)y;
    game->mouse.x = (float)x;
    game->mouse.y = (float)y;
}
