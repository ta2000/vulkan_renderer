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

struct QueueFamilyIndices
{
    int graphicsFamily;
    int presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR* formats;
    uint32_t presentModeCount;
    VkPresentModeKHR* presentModes;
};
void freeSwapChainSupportDetails(struct SwapChainSupportDetails* details);

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
    VkImage* swapChainImages;
    VkImageView* imageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    struct SwapChainSupportDetails swapChainDetails;

    // Render pass
    VkRenderPass renderPass;

    // Graphics pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Frame buffers
    VkFramebuffer* framebuffers;

    // Command pool/buffers
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;

    // Semaphores
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
};

void drawFrame(struct Engine* engine);

void EngineInit(struct Engine* self, GLFWwindow* window)
{
    self->physicalDevice = VK_NULL_HANDLE;
    self->device = VK_NULL_HANDLE;

    self->surfaceExtensionCount = 0;
    self->deviceExtensionCount = 0;

    self->framebuffers = NULL;
    self->commandBuffers = NULL;

    self->swapChainImages = NULL;
    self->imageViews = NULL;
    self->imageCount = 0;

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

    vkFreeCommandBuffers(
        self->device,
        self->commandPool,
        self->imageCount,
        self->commandBuffers
    );

    free(self->commandBuffers);

    // Destroy command pool
    vkDestroyCommandPool(self->device, self->commandPool, NULL);

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
    if (self->imageViews != NULL)
    {
        for (i=0; i<self->imageCount; i++)
        {
            vkDestroyImageView(self->device, self->imageViews[i], NULL);
        }
    }

    freeSwapChainSupportDetails(&(self->swapChainDetails));

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
	struct Engine* engine,
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
struct SwapChainSupportDetails querySwapChainSupport(
    struct Engine* engine,
    VkPhysicalDevice* physicalDevice
);
VkSurfaceFormatKHR renderer_get_image_format(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);
VkPresentModeKHR chooseSwapPresentMode(
    VkPresentModeKHR* availablePresentModes,
    int presentModeCount
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
void createSwapChain(struct Engine* engine);
VkSwapchainKHR renderer_get_swapchain(
	VkPhysicalDevice physical_device,
	VkDevice device,
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR image_format,
	VkExtent2D swapchain_extent,
	VkSwapchainKHR old_swapchain
);
uint32_t renderer_get_swapchain_image_count(
	VkDevice device,
	VkSwapchainKHR swapchain
);
void createImageViews(struct Engine* engine);
void createRenderPass(struct Engine* engine);
char* readFile(const char* fname, uint32_t* fsize);
void createGraphicsPipeline(struct Engine* engine);
void createShaderModule(
    struct Engine* engine,
    char* code,
    uint32_t codeSize,
    VkShaderModule* shaderModule
);
void createFrameBuffers(struct Engine* engine);
void createCommandPool(struct Engine* engine);
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
		engine,
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

    VkSurfaceFormatKHR surfaceFormat;
    surfaceFormat = renderer_get_image_format(
		engine->physicalDevice,
		engine->surface
    );
    engine->swapChainImageFormat = surfaceFormat.format;

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
        surfaceFormat,
        engine->swapChainExtent,
        VK_NULL_HANDLE
    );

    engine->imageCount = renderer_get_swapchain_image_count(
        engine->device,
        engine->swapChain
    );

    engine->swapChainImages = malloc(
        engine->imageCount * sizeof(*engine->swapChainImages)
    );
    vkGetSwapchainImagesKHR(
        engine->device,
        engine->swapChain,
        &engine->imageCount,
        engine->swapChainImages
    );

    createImageViews(engine);
    createRenderPass(engine);
    createGraphicsPipeline(engine);
    createFrameBuffers(engine);
    createCommandPool(engine);
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
		struct Engine* engine,
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

	querySwapChainSupport(engine, &physical_device_handle);

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

struct SwapChainSupportDetails querySwapChainSupport(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    struct SwapChainSupportDetails details;
    details.formats = NULL;
    details.presentModes = NULL;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        *physicalDevice,
        engine->surface,
        &(details.capabilities)
    );

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        *physicalDevice,
        engine->surface,
        &(details.formatCount),
        NULL
    );

    if (details.formatCount != 0)
    {
        details.formats = calloc(
            details.formatCount,
            sizeof(*(details.formats))
        );
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            *physicalDevice,
            engine->surface,
            &(details.formatCount),
            details.formats
        );
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *physicalDevice,
        engine->surface,
        &(details.presentModeCount),
        NULL
    );
    if (details.presentModeCount != 0)
    {
        details.presentModes = calloc(
            details.presentModeCount,
            sizeof(*(details.presentModes))
        );
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            *physicalDevice,
            engine->surface,
            &(details.presentModeCount),
            details.presentModes
        );
    }

    return details;
}

void freeSwapChainSupportDetails(struct SwapChainSupportDetails* details)
{
    if (details->formats) {
        free(details->formats);
    }
    if (details->presentModes) {
        free(details->presentModes);
    }
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

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, int presentModeCount)
{
    int i;
    for (i=0; i<presentModeCount; i++)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentModes[i];
    }

    return VK_PRESENT_MODE_FIFO_KHR;
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

void createImageViews(struct Engine* engine)
{
    engine->imageViews = calloc(
        engine->imageCount,
        sizeof(*(engine->imageViews))
    );

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.image = engine->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = engine->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result;
        result = vkCreateImageView(
            engine->device,
            &createInfo,
            NULL,
            &(engine->imageViews[i])
        );
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create image view %d.\n", i);
            exit(-1);
        }
    }
}

void createRenderPass(struct Engine* engine)
{
    VkAttachmentDescription colorAttachment;
    colorAttachment.flags = 0;
    colorAttachment.format = engine->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = NULL;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VkResult result;
    result = vkCreateRenderPass(
        engine->device,
        &renderPassCreateInfo,
        NULL,
        &(engine->renderPass)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create render pass.\n");
        exit(-1);
    }
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
        VkImageView attachments[] = { engine->imageViews[i] };

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

void createCommandPool(struct Engine* engine)
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0; // Relates to reset frequency of command buffers
    createInfo.queueFamilyIndex = engine->indices.graphicsFamily;

    VkResult result;
    result = vkCreateCommandPool(
        engine->device,
        &createInfo,
        NULL,
        &(engine->commandPool)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command pool.\n");
        exit(-1);
    }
}

void createCommandBuffers(struct Engine* engine)
{
    engine->commandBuffers = malloc(
            engine->imageCount * sizeof(*(engine->commandBuffers)));

    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.commandPool = engine->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = engine->imageCount;

    VkResult result;
    result = vkAllocateCommandBuffers(
        engine->device,
        &allocInfo,
        engine->commandBuffers
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command buffers.\n");
        exit(-1);
    }

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = NULL;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        vkBeginCommandBuffer(engine->commandBuffers[i], &beginInfo);

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
            engine->commandBuffers[i],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        vkCmdBindPipeline(
            engine->commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            engine->graphicsPipeline
        );

        vkCmdDraw(engine->commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(engine->commandBuffers[i]);

        VkResult result;
        result = vkEndCommandBuffer(engine->commandBuffers[i]);
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
    submitInfo.pCommandBuffers = &(engine->commandBuffers[imageIndex]);

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
