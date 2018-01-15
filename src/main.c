#include "renderer_buffer.h"
#include "renderer_image.h"
#include "renderer_tools.h"
#include "renderer.h"

#include <stdlib.h>

int main()
{
    struct renderer_resources* renderer_resources;
    renderer_resources = malloc(sizeof(*renderer_resources));

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Vulkan Window",
        NULL,
        NULL
    );
    glfwSetWindowUserPointer(window, renderer_resources);
    glfwSetWindowSizeCallback(window, renderer_resize);

    renderer_initialize_resources(renderer_resources, window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame(renderer_resources);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    renderer_destroy_resources(renderer_resources);
    free(renderer_resources);

    return 0;
}
