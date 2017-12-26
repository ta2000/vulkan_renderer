#include "renderer.h"

#include <stdlib.h>

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Vulkan Window",
        NULL,
        NULL
    );

    struct renderer_resources* renderer_resources;
    renderer_resources = malloc(sizeof(*renderer_resources));

    renderer_initialize_resources(renderer_resources, window);


    glfwDestroyWindow(window);
    glfwTerminate();

    free(renderer_resources);

    return 0;
}
