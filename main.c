#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define WIDTH 800
#define HEIGHT 600
#define VALIDATION_ENABLED 1

#define APP_NAME "Game"
#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct swapchain_buffer
{
    VkImage image;
    VkImageView image_view;
    VkCommandBuffer cmd;
};

struct QueueFamilyIndices
{
    int graphicsFamily;
    int presentFamily;
};

struct Engine
{
    // Window
    GLFWwindow* window;

    // Vulkan instance
    VkInstance instance;

    // Validation layers
    char* surfaceExtensions[64];
    uint32_t surfaceExtensionCount;
    char* deviceExtensions[64];
    uint32_t deviceExtensionCount;
    VkDebugReportCallbackEXT debugCallback;
    PFN_vkCreateDebugReportCallbackEXT createDebugCallback;
    PFN_vkDestroyDebugReportCallbackEXT destroyDebugCallback;

    // Surface
    VkSurfaceKHR surface;

    // Queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    struct QueueFamilyIndices indices;

    // Physical/logical device
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Swapchain/images
    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    struct swapchain_buffer* swapchain_buffers;
    VkSurfaceFormatKHR swapChainImageFormat;
    VkExtent2D swapChainExtent;

    // Render pass
    VkRenderPass renderPass;

    // Graphics pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Frame buffers
    VkFramebuffer* framebuffers;

    // Command pool
    VkCommandPool commandPool;

    // Semaphores
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
};

void drawFrame(struct Engine* engine);

void EngineInit(struct Engine* self, GLFWwindow* window)
{
    self->window = window;
}
void EngineRun(struct Engine* self)
{
    // GLFW main loop
    while(!glfwWindowShouldClose(self->window)) {
        glfwPollEvents();
        drawFrame(self);
    }

    vkDeviceWaitIdle(self->device);
}
void EngineDestroy(struct Engine* self)
{
    uint32_t i;

    vkDestroySemaphore(self->device, self->imageAvailable, NULL);
    vkDestroySemaphore(self->device, self->renderFinished, NULL);

    // Destroy frame buffers, same number as swapchain images
    if (self->framebuffers != NULL)
    {
        for (i=0; i<self->imageCount; i++)
        {
            vkDestroyFramebuffer(self->device, self->framebuffers[i], NULL);
        }
    }

    // Free extensions
    for (i=0; i<self->surfaceExtensionCount; i++)
    {
        free(self->surfaceExtensions[i]);
    }
    for (i=0; i<self->deviceExtensionCount; i++)
    {
        free(self->deviceExtensions[i]);
    }

    vkDestroyPipeline(self->device, self->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(self->device, self->pipelineLayout, NULL);
    vkDestroyRenderPass(self->device, self->renderPass, NULL);

    // Destroy all imageviews
    // TODO: Destroy as many image views as there are created
    // in case application fails in the middle of createing imageviews
    for (i=0; i<self->imageCount; i++)
    {
        vkDestroyImageView(self->device, self->swapchain_buffers[i].image_view, NULL);
        vkFreeCommandBuffers(
            self->device,
            self->commandPool,
            1,
            &self->swapchain_buffers[i].cmd
        );
    }

    // Destroy command pool
    vkDestroyCommandPool(self->device, self->commandPool, NULL);

    vkDestroySwapchainKHR(self->device, self->swapChain, NULL);
    vkDestroyDevice(self->device, NULL);
    vkDestroySurfaceKHR(self->instance, self->surface, NULL);
    self->destroyDebugCallback(
        self->instance,
        self->debugCallback,
        NULL
    );
    vkDestroyInstance(self->instance, NULL);
}

VkInstance renderer_get_instance();
_Bool checkValidationSupport(
    uint32_t validationLayerCount,
    const char** validationLayers
);
void getRequiredExtensions(struct Engine* engine);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData
);
void setupDebugCallback(struct Engine* engine);
VkSurfaceKHR renderer_get_surface(
	VkInstance instance,
	GLFWwindow* window
);
VkPhysicalDevice renderer_get_physical_device(
	VkInstance instance,
	VkSurfaceKHR surface,
	uint32_t device_extension_count,
	const char** device_extensions
);
bool physical_device_extensions_supported(
    VkPhysicalDevice physical_device,
    uint32_t required_extension_count,
    const char** required_extensions
);
VkSurfaceFormatKHR renderer_get_image_format(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);
VkExtent2D renderer_get_swapchain_extent(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    uint32_t window_width,
    uint32_t window_height
);
VkDevice renderer_get_device(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    VkPhysicalDeviceFeatures* required_features,
    uint32_t device_extension_count,
    const char** device_extensions
);
uint32_t renderer_get_graphics_queue(
	VkPhysicalDevice physical_device
);
uint32_t renderer_get_present_queue(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);
VkSwapchainKHR renderer_get_swapchain(
	VkPhysicalDevice physical_device,
	VkDevice device,
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR image_format,
	VkExtent2D swapchain_extent,
	VkSwapchainKHR old_swapchain
);
void renderer_create_swapchain_buffers(
    VkDevice device,
    VkCommandPool command_pool,
    VkSwapchainKHR swapchain,
    VkSurfaceFormatKHR swapchain_image_format,
    struct swapchain_buffer* swapchain_buffers,
    uint32_t swapchain_image_count
);
uint32_t renderer_get_swapchain_image_count(
	VkDevice device,
	VkSwapchainKHR swapchain
);
void createImageViews(struct Engine* engine);
VkRenderPass renderer_get_render_pass(
	VkDevice device,
	VkFormat image_format
	//VkFormat depth_format
);
char* readFile(const char* fname, uint32_t* fsize);
void createGraphicsPipeline(struct Engine* engine);

size_t renderer_get_file_size(
    const char* fname
);

void renderer_read_file_to_buffer(
    const char* fname,
    char** buffer,
    size_t buffer_size
);

VkShaderModule renderer_get_shader_module(
    VkDevice device,
    char* code,
    size_t code_size
);

VkPipelineLayout renderer_get_pipeline_layout(
    VkDevice device,
    VkDescriptorSetLayout* descriptor_layouts,
    uint32_t descriptor_layout_count,
    VkPushConstantRange* push_constant_ranges,
    uint32_t push_constant_range_count
);

VkPipeline renderer_get_graphics_pipeline(
    VkDevice device,
    VkShaderModule* shader_modules,
    VkShaderStageFlagBits* shader_stage_flags,
    uint32_t shader_stage_count,
    VkExtent2D swapchain_extent,
    VkPipelineLayout pipeline_layout,
    VkRenderPass render_pass,
    uint32_t subpass
);

void createShaderModule(
    struct Engine* engine,
    char* code,
    uint32_t codeSize,
    VkShaderModule* shaderModule
);
void createFrameBuffers(struct Engine* engine);
VkCommandPool renderer_get_command_pool(
    VkPhysicalDevice physical_device,
    VkDevice device
);
void createCommandBuffers(struct Engine* engine);
void createSemaphores(struct Engine* engine);

uint32_t min(uint32_t a, uint32_t b)
{
    return (a < b ? a : b);
}

uint32_t max(uint32_t a, uint32_t b)
{
    return (a > b ? a : b);
}

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Vulkan Window",
        NULL,
        NULL
    );

    struct Engine* engine = calloc(1, sizeof(*engine));
    EngineInit(engine, window);

    engine->instance = renderer_get_instance(engine);

    setupDebugCallback(engine);
    engine->surface = renderer_get_surface(
        engine->instance,
        window
    );

    engine->deviceExtensions[engine->deviceExtensionCount] =
        calloc(1, strlen(VK_KHR_SWAPCHAIN_EXTENSION_NAME)+1);
    strcpy(
        engine->deviceExtensions[engine->deviceExtensionCount++],
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    );

    engine->physicalDevice = renderer_get_physical_device(
        engine->instance,
        engine->surface,
        engine->deviceExtensionCount,
        (const char**)engine->deviceExtensions
    );

	engine->indices.graphicsFamily = renderer_get_graphics_queue(
		engine->physicalDevice
    );
    engine->indices.presentFamily = renderer_get_present_queue(
        engine->physicalDevice,
        engine->surface
    );

    engine->device = renderer_get_device(
        engine->physicalDevice,
        engine->surface,
        NULL,
        engine->deviceExtensionCount,
        (const char**)engine->deviceExtensions
    );

    vkGetDeviceQueue(
        engine->device,
        engine->indices.graphicsFamily,
        0,
        &(engine->graphicsQueue)
    );
    vkGetDeviceQueue(
        engine->device,
        engine->indices.presentFamily,
        0,
        &(engine->presentQueue)
    );

    engine->swapChainImageFormat = renderer_get_image_format(
		engine->physicalDevice,
		engine->surface
    );

    engine->swapChainExtent = renderer_get_swapchain_extent(
        engine->physicalDevice,
        engine->surface,
        WIDTH,
        HEIGHT
    );

    engine->swapChain = renderer_get_swapchain(
        engine->physicalDevice,
        engine->device,
        engine->surface,
        engine->swapChainImageFormat,
        engine->swapChainExtent,
        VK_NULL_HANDLE
    );

    engine->commandPool = renderer_get_command_pool(
        engine->physicalDevice,
        engine->device
    );

    engine->imageCount = renderer_get_swapchain_image_count(
        engine->device,
        engine->swapChain
    );

    engine->swapchain_buffers = malloc(
        engine->imageCount * sizeof(*engine->swapchain_buffers)
    );

    renderer_create_swapchain_buffers(
        engine->device,
        engine->commandPool,
        engine->swapChain,
        engine->swapChainImageFormat,
        engine->swapchain_buffers,
        engine->imageCount
    );

	engine->renderPass = renderer_get_render_pass(
		engine->device,
		engine->swapChainImageFormat.format
	);

    engine->pipelineLayout = renderer_get_pipeline_layout(
        engine->device,
        NULL,
        0,
        NULL,
        0
    );

    VkShaderModule vert_shader_module;
    size_t vert_shader_size = renderer_get_file_size("shaders/vert.spv");
    char* vert_shader_code = malloc(vert_shader_size);
    renderer_read_file_to_buffer("shaders/vert.spv", &vert_shader_code, vert_shader_size);
    vert_shader_module = renderer_get_shader_module(
        engine->device,
        vert_shader_code,
        vert_shader_size
    );

    VkShaderModule frag_shader_module;
    size_t frag_shader_size = renderer_get_file_size("shaders/frag.spv");
    char* frag_shader_code = malloc(frag_shader_size);
    renderer_read_file_to_buffer("shaders/frag.spv", &frag_shader_code, frag_shader_size);
    frag_shader_module = renderer_get_shader_module(
        engine->device,
        frag_shader_code,
        frag_shader_size
    );

    VkShaderModule shader_modules[] = {vert_shader_module, frag_shader_module};

    VkShaderStageFlagBits shader_stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    engine->graphicsPipeline = renderer_get_graphics_pipeline(
        engine->device,
        shader_modules,
        shader_stages,
        2,
        engine->swapChainExtent,
        engine->pipelineLayout,
        engine->renderPass,
        0
    );

    free(vert_shader_code);
    free(frag_shader_code);
    vkDestroyShaderModule(engine->device, vert_shader_module, NULL);
    vkDestroyShaderModule(engine->device, frag_shader_module, NULL);

    //createGraphicsPipeline(engine);

    createFrameBuffers(engine);
    createCommandBuffers(engine);
    createSemaphores(engine);

    EngineRun(engine);

    EngineDestroy(engine);

    free(engine);

    // Close window
    glfwDestroyWindow(window);

    // Stop GLFW
    glfwTerminate();

    return 0;
}

VkInstance renderer_get_instance()
{
    VkInstance instance_handle;
    instance_handle = VK_NULL_HANDLE;

    // Create info
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    // Application info
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = APP_NAME,
        .applicationVersion = VK_MAKE_VERSION(
                APP_VERSION_MAJOR,
                APP_VERSION_MINOR,
                APP_VERSION_PATCH),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0,0,0),
        .apiVersion = VK_API_VERSION_1_0
    };
    create_info.pApplicationInfo = &app_info;

    // Validation layers
    const char* enabled_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
    uint32_t enabled_layer_count = 1;

    VkLayerProperties* available_layers;
    uint32_t available_layer_count;

    vkEnumerateInstanceLayerProperties(&available_layer_count, NULL);
    assert(available_layer_count > 0);

    available_layers = malloc(
        available_layer_count * sizeof(*available_layers)
    );
    assert(available_layers);

    vkEnumerateInstanceLayerProperties(
        &available_layer_count,
        available_layers
    );

    uint32_t i, j;
    bool layer_found = true;
    for (i = 0; i < enabled_layer_count; i++)
    {
        if (VALIDATION_ENABLED)
            break;

        layer_found = false;
        for (j = 0; j < available_layer_count; j++)
        {
            if (!strcmp(enabled_layers[i], available_layers[j].layerName))
            {
                layer_found = true;
                break;
            }
        }

        if (layer_found)
            break;
    }
    assert(layer_found);

    if (VALIDATION_ENABLED) {
        create_info.enabledLayerCount = enabled_layer_count;
        create_info.ppEnabledLayerNames = enabled_layers;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = NULL;
    }

    // Extensions
    const char** glfw_extensions;
    uint32_t glfw_extension_count;
    glfw_extensions = glfwGetRequiredInstanceExtensions(
        &glfw_extension_count
    );

    const char* my_extensions[] = {VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
    uint32_t my_extension_count = 1;

    char** all_extensions;
    uint32_t all_extension_count = glfw_extension_count + my_extension_count;
    all_extensions = malloc(all_extension_count * sizeof(char*));

    for (i=0; i<glfw_extension_count; i++)
    {
        all_extensions[i] = calloc(1, strlen(glfw_extensions[i]) + 1);
        strcpy(all_extensions[i], glfw_extensions[i]);
    }
    for (j=0; j<my_extension_count; j++)
    {
        all_extensions[i+j] = calloc(1, strlen(my_extensions[j]) + 1);
        strcpy(all_extensions[i+j], my_extensions[j]);
    }

    create_info.enabledExtensionCount = all_extension_count;
    create_info.ppEnabledExtensionNames = (const char* const*)all_extensions;

    // Create instance
    VkResult result;
    result = vkCreateInstance(
        &create_info,
        NULL,
        &instance_handle
    );
    assert(result == VK_SUCCESS);

    for (i=0; i<all_extension_count; i++)
        free(all_extensions[i]);
    free(all_extensions);
    free(available_layers);

    return instance_handle;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* pLayerPrefix,
    const char* pMsg,
    void* pUserData)
{
    char* message = malloc(strlen(pMsg) + 100);
    assert(message);

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        sprintf(message, "%s error, code %d: %s",
                pLayerPrefix, code, pMsg);
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        sprintf(message, "%s warning, code %d: %s",
                pLayerPrefix, code, pMsg);
    else
        return VK_FALSE;

    fprintf(stderr, "%s\n", message);

    free(message);

    return VK_FALSE;
}

void setupDebugCallback(struct Engine* engine)
{
    if (!VALIDATION_ENABLED) return;

    VkDebugReportCallbackCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = &debugCallback;
    createInfo.pUserData = NULL;

    engine->createDebugCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            engine->instance,
            "vkCreateDebugReportCallbackEXT"
        );
    if (!engine->createDebugCallback)
    {
        fprintf(stderr, "GetProcAddr vkCreateDebugReportCallback failed.\n");
        exit(-1);
    }

    engine->destroyDebugCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            engine->instance,
            "vkDestroyDebugReportCallbackEXT"
        );
    if (!engine->destroyDebugCallback)
    {
        fprintf(stderr, "GetProcAddr vkDestroyDebugReportCallback failed.\n");
        exit(-1);
    }

    VkResult result = engine->createDebugCallback(
        engine->instance,
        &createInfo,
        NULL,
        &(engine->debugCallback)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to set up debug callback.\n");
        exit(-1);
    }
}

VkSurfaceKHR renderer_get_surface(
        VkInstance instance,
        GLFWwindow* window)
{
    VkSurfaceKHR surface_handle;

    VkResult result;
    result = glfwCreateWindowSurface(
        instance,
        window,
        NULL,
        &surface_handle
    );
    assert(result == VK_SUCCESS);

    return surface_handle;
}

VkPhysicalDevice renderer_get_physical_device(
        VkInstance instance,
        VkSurfaceKHR surface,
        uint32_t device_extension_count,
        const char** device_extensions)
{
    VkPhysicalDevice physical_device_handle;
    physical_device_handle = VK_NULL_HANDLE;

    // Enumerate devices
    VkPhysicalDevice* physical_devices;
    uint32_t physical_device_count;

    vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);
    assert(physical_device_count > 0);

    physical_devices = malloc(
        physical_device_count * sizeof(*physical_devices)
    );
    assert(physical_devices);

    vkEnumeratePhysicalDevices(
        instance,
        &physical_device_count,
        physical_devices
    );

    uint32_t i;
    for (i=0; i<physical_device_count; i++)
    {
        // Ensure required extensions are supported
        if (physical_device_extensions_supported(
                physical_devices[i],
                device_extension_count,
                device_extensions))
        {
            printf("Extensions not supported\n");
            continue;
        }

        // Ensure there is at least one surface format
        // compatible with the surface
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_devices[i],
            surface,
            &format_count,
            NULL
        );
        if (format_count < 1) {
            printf("No surface formats available\n");
            continue;
        }

        // Ensured there is at least one present mode available
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_devices[i],
            surface,
            &present_mode_count,
            NULL
        );
        if (present_mode_count < 1) {
            printf("No present modes available for surface\n");
            continue;
        }

        // Ensure the physical device has WSI
        VkQueueFamilyProperties* queue_family_properties;
        uint32_t queue_family_count;

        vkGetPhysicalDeviceQueueFamilyProperties(
            physical_devices[i],
            &queue_family_count,
            NULL
        );

        queue_family_properties = malloc(
            queue_family_count * sizeof(*queue_family_properties)
        );

        vkGetPhysicalDeviceQueueFamilyProperties(
            physical_devices[i],
            &queue_family_count,
            queue_family_properties
        );

        uint32_t j;
        VkBool32 wsi_support = VK_FALSE;
        VkBool32 graphics_bit = VK_FALSE;
        for (j=0; j<queue_family_count; j++)
        {
            graphics_bit =
                queue_family_properties[j].queueCount > 0 &&
                queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT;

            VkResult wsi_query_result;
            wsi_query_result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physical_devices[i],
                j,
                surface,
                &wsi_support
            );
            assert(wsi_query_result == VK_SUCCESS);

            if (wsi_support && graphics_bit)
                break;
        }
        if (!(wsi_support && graphics_bit)) {
            printf("Suitable GPU not found:\n");
            printf("Window System Integration: %d\n", wsi_support);
            printf("Graphical operations supported: %d\n", graphics_bit);
            continue;
        }

        free(queue_family_properties);

        physical_device_handle = physical_devices[i];
        break;
    }

    return physical_device_handle;
}

bool physical_device_extensions_supported(
    VkPhysicalDevice physical_device,
    uint32_t required_extension_count,
    const char** required_extensions)
{
    // Enumerate extensions for each device
    VkExtensionProperties* available_extensions;
    uint32_t available_extension_count;

    vkEnumerateDeviceExtensionProperties(
        physical_device,
        NULL,
        &available_extension_count,
        NULL
    );
    assert(available_extension_count > 0);

    available_extensions = malloc(
        available_extension_count * sizeof(*available_extensions)
    );
    assert(available_extensions);

    vkEnumerateDeviceExtensionProperties(
        physical_device,
        NULL,
        &available_extension_count,
        available_extensions
    );

    // Determine if device's extensions contain necessary extensions
    uint32_t i, j;
    for (i=0; i<required_extension_count; i++)
    {
        for (j=0; j<available_extension_count; j++)
        {
            if (strcmp(
                    required_extensions[i],
                    available_extensions[j].extensionName
                ))
            {
                return false;
            }
        }
    }

    return true;
}

VkDevice renderer_get_device(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        VkPhysicalDeviceFeatures* required_features,
        uint32_t device_extension_count,
        const char** device_extensions)
{
    VkDevice device_handle;
    device_handle = VK_NULL_HANDLE;

    uint32_t graphics_family_index = renderer_get_graphics_queue(
        physical_device
    );
    uint32_t present_family_index = renderer_get_present_queue(
        physical_device,
        surface
    );

    uint32_t device_queue_count = 2;
    uint32_t device_queue_indices[] = {
        graphics_family_index,
        present_family_index
    };
    float device_queue_priorities[] = {1.0f, 1.0f};
    VkDeviceQueueCreateFlags device_queue_flags[] = {0, 0};

    VkDeviceQueueCreateInfo* device_queue_infos;
    device_queue_infos = malloc(
        sizeof(*device_queue_infos) * device_queue_count
    );

    uint32_t i;
    for (i=0; i<device_queue_count; i++) {
        VkDeviceQueueCreateInfo device_queue_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = device_queue_flags[i],
            .queueFamilyIndex = device_queue_indices[i],
            .queueCount = 1,
            .pQueuePriorities = &device_queue_priorities[i]
        };
        device_queue_infos[i] = device_queue_info;
    }

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pQueueCreateInfos = device_queue_infos,
        .pEnabledFeatures = required_features,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = device_extension_count,
        .ppEnabledExtensionNames = (const char* const*)device_extensions
    };

    if (graphics_family_index == present_family_index)
        device_info.queueCreateInfoCount = 1;
    else
        device_info.queueCreateInfoCount = 2;

    VkResult result;
    result = vkCreateDevice(
        physical_device,
        &device_info,
        NULL,
        &device_handle
    );
    assert(result == VK_SUCCESS);

    free(device_queue_infos);

    return device_handle;
}

uint32_t renderer_get_graphics_queue(
        VkPhysicalDevice physical_device)
{
    uint32_t graphics_queue_index;

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        NULL
    );

    VkQueueFamilyProperties* queue_family_properties;
    queue_family_properties = malloc(
        queue_family_count * sizeof(*queue_family_properties)
    );

    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        queue_family_properties
    );

    uint32_t i;
    bool graphics_queue_found = false;
    for (i=0; i<queue_family_count; i++)
    {
        if (queue_family_properties[i].queueCount > 0 &&
            queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_index = i;
            graphics_queue_found = true;
            break;
        }
    }
    assert(graphics_queue_found);

    free(queue_family_properties);

    return graphics_queue_index;
}

uint32_t renderer_get_present_queue(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface)
{
    uint32_t present_queue_index;

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        NULL
    );

    VkQueueFamilyProperties* queue_family_properties;
    queue_family_properties = malloc(
        queue_family_count * sizeof(*queue_family_properties)
    );

    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        queue_family_properties
    );

    uint32_t i;
    VkBool32 wsi_support;
    bool present_queue_found = false;
    for (i=0; i<queue_family_count; i++)
    {
        VkResult wsi_query_result;
        wsi_query_result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physical_device,
            i,
            surface,
            &wsi_support
        );
        assert(wsi_query_result == VK_SUCCESS);

        if (wsi_support) {
            present_queue_index = i;
            present_queue_found = true;
            break;
        }
    }
    assert(present_queue_found);

    free(queue_family_properties);

    return present_queue_index;
}

VkSwapchainKHR renderer_get_swapchain(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkSurfaceKHR surface,
        VkSurfaceFormatKHR image_format,
        VkExtent2D swapchain_extent,
        VkSwapchainKHR old_swapchain)
{
    VkSwapchainKHR swapchain_handle;
    swapchain_handle = VK_NULL_HANDLE;

    // Determine image count (maxImageCount 0 == no limit)
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkResult result;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &surface_capabilities
    );
    assert(result == VK_SUCCESS);

    uint32_t desired_image_count;
    desired_image_count = surface_capabilities.minImageCount + 1;
    if (desired_image_count > 0 &&
        desired_image_count > surface_capabilities.maxImageCount)
    {
        desired_image_count = surface_capabilities.maxImageCount;
    }

    // Queue family indices
    VkSharingMode sharing_mode;

    uint32_t queue_family_count;
    const uint32_t* queue_family_indices;

    uint32_t graphics_family_index = renderer_get_graphics_queue(
        physical_device
    );
    uint32_t present_family_index = renderer_get_present_queue(
        physical_device,
        surface
    );

    if (graphics_family_index != present_family_index)
    {
        sharing_mode = VK_SHARING_MODE_CONCURRENT;
        queue_family_count = 2;
        const uint32_t indices[] = {
            graphics_family_index,
            present_family_index
        };
        queue_family_indices = indices;
    }
    else
    {
        sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        queue_family_count = 0;
        queue_family_indices = NULL;
    }

    // Find a present mode, ideally MAILBOX unless unavailable,
    // in which case fall back to FIFO (always available)
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &present_mode_count,
        NULL
    );

    present_modes = malloc(present_mode_count * sizeof(*present_modes));

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &present_mode_count,
        present_modes
    );

    uint32_t i;
    for (i=0; i<present_mode_count; i++)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = present_modes[i];
            break;
        }
    }

    free(present_modes);

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = surface,
        .minImageCount = desired_image_count,
        .imageFormat = image_format.format,
        .imageColorSpace = image_format.colorSpace,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = sharing_mode,
        .queueFamilyIndexCount = queue_family_count,
        .pQueueFamilyIndices = queue_family_indices,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain
    };

    result = vkCreateSwapchainKHR(
        device,
        &swapchain_info,
        NULL,
        &swapchain_handle
    );
    switch (result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            printf("Out of host memory!\n");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            printf("Out of device memory!\n");
            break;
        case VK_ERROR_DEVICE_LOST:
            printf("Device lost!\n");
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            printf("Surface lost!\n");
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            printf("Native window in use!\n");
            break;
        default:
            break;
    }
    assert(result == VK_SUCCESS);

    if (old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(
            device,
            old_swapchain,
            NULL
        );
    }

    return swapchain_handle;
}

uint32_t renderer_get_swapchain_image_count(
        VkDevice device,
        VkSwapchainKHR swapchain)
{
    uint32_t image_count;
    vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &image_count,
        NULL
    );
    return image_count;
}

VkSurfaceFormatKHR renderer_get_image_format(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface)
{
    VkSurfaceFormatKHR image_format;

    uint32_t format_count;
    VkSurfaceFormatKHR* formats;
    VkResult result;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &format_count,
        NULL
    );
    assert(result == VK_SUCCESS);
    formats = malloc(format_count * sizeof(*formats));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &format_count,
        formats
    );
    assert(result == VK_SUCCESS);

    // Free to choose any format
    if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        image_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    // Limited selection of formats
    else
    {
        uint32_t i;
        bool ideal_format_found = false;
        for (i=0; i<format_count; i++)
        {
            // Ideal format B8G8R8A8_UNORM, SRGB_NONLINEAR
            if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
                formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                image_format = formats[i];
                ideal_format_found = true;
                break;
            }
        }

        if (!ideal_format_found)
        {
            image_format = formats[0];
        }
    }

    free(formats);

    return image_format;
}

VkExtent2D renderer_get_swapchain_extent(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        uint32_t window_width,
        uint32_t window_height)
{
    // Determine extent (resolution of swapchain images)
    VkExtent2D extent;
    extent.width = window_width;
    extent.height = window_height;

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkResult result;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &surface_capabilities
    );
    assert(result == VK_SUCCESS);

    if (surface_capabilities.currentExtent.width == UINT32_MAX)
    {
        extent.width = MAX(
            surface_capabilities.minImageExtent.width,
            MIN(surface_capabilities.maxImageExtent.width, extent.width)
        );

        extent.height = MAX(
            surface_capabilities.minImageExtent.height,
            MIN(surface_capabilities.maxImageExtent.height, extent.height)
        );
    }

    return extent;
}

void renderer_create_swapchain_buffers(
        VkDevice device,
        VkCommandPool command_pool,
        VkSwapchainKHR swapchain,
        VkSurfaceFormatKHR swapchain_image_format,
        struct swapchain_buffer* swapchain_buffers,
        uint32_t swapchain_image_count)
{
    VkImage* images;
    images = malloc(swapchain_image_count * sizeof(*images));
    assert(images);

    VkResult result;
    result = vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &swapchain_image_count,
        images
    );
    assert(result == VK_SUCCESS);

    VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .format = swapchain_image_format.format,
        .components = {
             .r = VK_COMPONENT_SWIZZLE_R,
             .g = VK_COMPONENT_SWIZZLE_G,
             .b = VK_COMPONENT_SWIZZLE_B,
             .a = VK_COMPONENT_SWIZZLE_A
        },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .viewType = VK_IMAGE_VIEW_TYPE_2D
    };

    uint32_t i;
    for (i=0; i<swapchain_image_count; i++)
    {
        result = vkAllocateCommandBuffers(
            device,
            &cmd_alloc_info,
            &swapchain_buffers[i].cmd
        );
        assert(result == VK_SUCCESS);

        swapchain_buffers[i].image = images[i];
        view_info.image = swapchain_buffers[i].image;

        result = vkCreateImageView(
            device,
            &view_info,
            NULL,
            &swapchain_buffers[i].image_view
        );
        assert(result == VK_SUCCESS);
    }
}

VkRenderPass renderer_get_render_pass(
        VkDevice device,
        VkFormat image_format)
        //VkFormat depth_format)
{
    VkRenderPass render_pass_handle;
    render_pass_handle = VK_NULL_HANDLE;

    VkAttachmentDescription color_desc = {
        .flags = 0,
        .format = image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference color_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    /*VkAttachmentDescription depth_desc = {
        .flags = 0,
        .format = depth_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depth_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };*/

    VkAttachmentDescription attachments[] = {color_desc/*, depth_desc*/};

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = NULL, //&depth_ref,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL
    };

    VkSubpassDependency subpass_dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 1, //2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency
    };

    VkResult result;
    result = vkCreateRenderPass(
        device,
        &render_pass_info,
        NULL,
        &render_pass_handle
    );
    assert(result == VK_SUCCESS);

    return render_pass_handle;
}

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

VkPipelineLayout renderer_get_pipeline_layout(
        VkDevice device,
        VkDescriptorSetLayout* descriptor_layouts,
        uint32_t descriptor_layout_count,
        VkPushConstantRange* push_constant_ranges,
        uint32_t push_constant_range_count)
{
    VkPipelineLayout pipeline_layout_handle;
    pipeline_layout_handle = VK_NULL_HANDLE;

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = descriptor_layout_count,
        .pSetLayouts = descriptor_layouts,
        .pushConstantRangeCount = push_constant_range_count,
        .pPushConstantRanges = push_constant_ranges
    };

    VkResult result;
    result = vkCreatePipelineLayout(
        device,
        &layout_info,
        NULL,
        &pipeline_layout_handle
    );
    assert(result == VK_SUCCESS);

    return pipeline_layout_handle;
}

VkShaderModule renderer_get_shader_module(
        VkDevice device,
        char* code,
        size_t code_size)
{
    VkShaderModule shader_module_handle;

    VkShaderModuleCreateInfo shader_module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = code_size,
        .pCode = (uint32_t*)code
    };

    VkResult result;
    result = vkCreateShaderModule(
        device,
        &shader_module_info,
        NULL,
        &shader_module_handle
    );
    assert(result == VK_SUCCESS);

    return shader_module_handle;
}

VkPipeline renderer_get_graphics_pipeline(
        VkDevice device,
        VkShaderModule* shader_modules,
        VkShaderStageFlagBits* shader_stage_flags,
        uint32_t shader_stage_count,
        VkExtent2D swapchain_extent,
        VkPipelineLayout pipeline_layout,
        VkRenderPass render_pass,
        uint32_t subpass)
{
    VkPipelineShaderStageCreateInfo* shader_infos;
    shader_infos = malloc(shader_stage_count * sizeof(*shader_infos));

    uint32_t i;
    for (i=0; i<shader_stage_count; i++) {
        shader_infos[i].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_infos[i].pNext = NULL;
        shader_infos[i].flags = 0;
        shader_infos[i].stage = shader_stage_flags[i];
        shader_infos[i].module = shader_modules[i];
        shader_infos[i].pName = "main";
        shader_infos[i].pSpecializationInfo = NULL;
    }

    /*VkVertexInputBindingDescription binding_description = {
        .binding = 0,
        .stride = sizeof(struct renderer_vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription position_attribute_description = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(struct renderer_vertex, x)
    };

    VkVertexInputAttributeDescription texture_attribute_description = {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(struct renderer_vertex, u)
    };

    VkVertexInputAttributeDescription attribute_descriptions[] = {
        position_attribute_description,
        texture_attribute_description
    };*/

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO/*,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attribute_descriptions*/
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

	VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = (float)swapchain_extent.width,
        .height = (float)swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapchain_extent
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front.failOp = 0,
        .front.passOp = 0,
        .front.depthFailOp = 0,
        .front.compareOp = 0,
        .front.compareMask = 0,
        .front.writeMask = 0,
        .front.reference = 0,
        .back.failOp = 0,
        .back.passOp = 0,
        .back.depthFailOp = 0,
        .back.compareOp = 0,
        .back.compareMask = 0,
        .back.writeMask = 0,
        .back.reference = 0,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = 0,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stageCount = shader_stage_count,
        .pStages = shader_infos,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = NULL,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = &depth_stencil_state,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = NULL,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .subpass = subpass,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VkPipeline graphics_pipeline_handle;
    graphics_pipeline_handle = VK_NULL_HANDLE;

    VkResult result;
    result = vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &graphics_pipeline_info,
        NULL,
        &graphics_pipeline_handle
    );
    assert(result == VK_SUCCESS);

    free(shader_infos);

    return graphics_pipeline_handle;
}

void createGraphicsPipeline(struct Engine* engine)
{
    char* vertShaderFname = "shaders/vert.spv";
    char* fragShaderFname = "shaders/frag.spv";
    char* vertShader = NULL; uint32_t vertShaderSize;
    char* fragShader = NULL; uint32_t fragShaderSize;

    vertShader = readFile(vertShaderFname, &vertShaderSize);
    if (!vertShader)
    {
        fprintf(stderr, "Reading file %s failed.\n", vertShaderFname);
        exit(-1);
    }

    fragShader = readFile(fragShaderFname, &fragShaderSize);
    if (!fragShader)
    {
        fprintf(stderr, "Reading file %s failed.\n", fragShaderFname);
        free(vertShader);
        exit(-1);
    }

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    createShaderModule(engine, vertShader, vertShaderSize, &vertShaderModule);
    createShaderModule(engine, fragShader, fragShaderSize, &fragShaderModule);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    VkGraphicsPipelineCreateInfo pipelineInfo;
    VkPipelineShaderStageCreateInfo shaderStageInfos[2];
    VkPipelineVertexInputStateCreateInfo vertInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    //VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    //VkDynamicState dynamicStates[2];
    //VkPipelineDynamicStateCreateInfo dyanamicStateInfo;

    /*memset(&dynamicStates, 0, sizeof(dynamicStates));
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_LINE_WIDTH;
    memset(&dynamicStateInfo, 0, sizeof(dynamicStateInfo));
    dynamicStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;*/

    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.stageCount = 2;

    memset(shaderStageInfos, 0, 2*sizeof(shaderStageInfos[0]));
    shaderStageInfos[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[0].pNext = NULL;
    shaderStageInfos[0].flags = 0;
    shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfos[0].module = vertShaderModule;
    shaderStageInfos[0].pName = "main";
    shaderStageInfos[0].pSpecializationInfo = NULL;
    shaderStageInfos[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[1].pNext = NULL;
    shaderStageInfos[1].flags = 0;
    shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageInfos[1].module = fragShaderModule;
    shaderStageInfos[1].pName = "main";
    shaderStageInfos[1].pSpecializationInfo = NULL;

    memset(&vertInputInfo, 0, sizeof(vertInputInfo));
    vertInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    memset(&inputAssemblyInfo, 0, sizeof(inputAssemblyInfo));
    inputAssemblyInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    memset(&rasterizationInfo, 0, sizeof(rasterizationInfo));
    rasterizationInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor = 0.0f;
    rasterizationInfo.depthBiasClamp = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationInfo.lineWidth = 1.0f;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) engine->swapChainExtent.width;
    viewport.height = (float) engine->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = engine->swapChainExtent;

    memset(&viewportInfo, 0, sizeof(viewportInfo));
    viewportInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    memset(&multisampleInfo, 0, sizeof(multisampleInfo));
    multisampleInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.minSampleShading = 0.0f;
    multisampleInfo.pSampleMask = NULL;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    memset(&colorBlendAttachment, 0, sizeof(colorBlendAttachment));
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    memset(&colorBlendInfo, 0, sizeof(colorBlendInfo));
    colorBlendInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    colorBlendInfo.blendConstants[0] = 0.0f;
    colorBlendInfo.blendConstants[1] = 0.0f;
    colorBlendInfo.blendConstants[2] = 0.0f;
    colorBlendInfo.blendConstants[3] = 0.0f;

    if (vkCreatePipelineLayout(
            engine->device,
            &pipelineLayoutInfo,
            NULL,
            &(engine->pipelineLayout)
    ) != VK_SUCCESS )
    {
        fprintf(stderr, "Failed to create pipeline layout.\n");
        exit(-1);
    }

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageInfos;
    pipelineInfo.pVertexInputState = &vertInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pTessellationState = NULL;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = NULL;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.renderPass = engine->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(
            engine->device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            NULL,
            &(engine->graphicsPipeline)
    ) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create graphics pipeline.\n");
        exit(-1);
    }

    // Free memory, shader modules no longer needed
    free(vertShader);
    free(fragShader);
    vkDestroyShaderModule(engine->device, vertShaderModule, NULL);
    vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
}

char* readFile(const char* fname, uint32_t* fsize)
{
    FILE *fp = fopen(fname, "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to load file %s.\n", fname);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    uint32_t length = ftell(fp);
    rewind(fp);

    char* buffer = malloc(length);
    fread(buffer, length, 1, fp);
    *fsize = length;

    fclose(fp);

    return buffer;
}

void createShaderModule(struct Engine* engine, char* code, uint32_t codeSize, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (uint32_t*)code;

    VkResult result;
    result = vkCreateShaderModule(
        engine->device,
        &createInfo,
        NULL,
        shaderModule
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Shader module creation failed.\n");
        exit(-1);
    }
}

void createFrameBuffers(struct Engine* engine)
{
    engine->framebuffers = malloc(
            engine->imageCount * sizeof(*(engine->framebuffers)));

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkImageView attachments[] = {engine->swapchain_buffers[i].image_view};

        VkFramebufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.renderPass = engine->renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = engine->swapChainExtent.width;
        createInfo.height = engine->swapChainExtent.height;
        createInfo.layers = 1;

        VkResult result;
        result = vkCreateFramebuffer(
            engine->device,
            &createInfo,
            NULL,
            &(engine->framebuffers[i])
        );

        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Error during framebuffer creation.\n");
            exit(-1);
        }
    }
}

VkCommandPool renderer_get_command_pool(
        VkPhysicalDevice physical_device,
        VkDevice device)
{
    VkCommandPool command_pool_handle;
    command_pool_handle = VK_NULL_HANDLE;

    uint32_t graphics_family_index = renderer_get_graphics_queue(
        physical_device
    );

    VkCommandPoolCreateInfo command_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = graphics_family_index
    };

    VkResult result;
    result = vkCreateCommandPool(
        device,
        &command_pool_info,
        NULL,
        &command_pool_handle
    );
    assert(result == VK_SUCCESS);

    return command_pool_handle;
}

void createCommandBuffers(struct Engine* engine)
{
    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = NULL;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        vkBeginCommandBuffer(engine->swapchain_buffers[i].cmd, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = NULL;
        renderPassInfo.renderPass = engine->renderPass;
        renderPassInfo.framebuffer = engine->framebuffers[i];
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent = engine->swapChainExtent;

        VkClearValue clearColor = {
            .color.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
        };

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(
            engine->swapchain_buffers[i].cmd,
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        vkCmdBindPipeline(
            engine->swapchain_buffers[i].cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            engine->graphicsPipeline
        );

        vkCmdDraw(engine->swapchain_buffers[i].cmd, 3, 1, 0, 0);

        vkCmdEndRenderPass(engine->swapchain_buffers[i].cmd);

        VkResult result;
        result = vkEndCommandBuffer(engine->swapchain_buffers[i].cmd);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to record command buffers.\n");
            exit(-1);
        }
    }
}

void createSemaphores(struct Engine* engine)
{
    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;

    VkResult result;
    result = vkCreateSemaphore(
        engine->device,
        &createInfo,
        NULL,
        &(engine->imageAvailable)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create semaphore.\n");
        exit(-1);
    }

    result = vkCreateSemaphore(
        engine->device,
        &createInfo,
        NULL,
        &(engine->renderFinished)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create semaphore.\n");
        exit(-1);
    }
}

void drawFrame(struct Engine* engine)
{
    uint32_t imageIndex;
    vkAcquireNextImageKHR(
        engine->device,
        engine->swapChain,
        UINT64_MAX, // Wait for next image indefinitely (ns)
        engine->imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[] = { engine->imageAvailable };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(engine->swapchain_buffers[imageIndex].cmd);

    VkSemaphore signalSemaphores[] = { engine->renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult result;
    result = vkQueueSubmit(
        engine->graphicsQueue,
        1,
        &submitInfo,
        VK_NULL_HANDLE
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Error while submitting queue.\n");
        exit(-1);
    }

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { engine->swapChain };

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(engine->presentQueue, &presentInfo);
}
