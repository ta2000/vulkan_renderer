#include "renderer_image.h"
#include "renderer_buffer.h"
#include "renderer_tools.h"
#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

void renderer_initialize_resources(
        struct renderer_resources* resources,
        GLFWwindow* window)
{
    memset(resources, 0, sizeof(*resources));

    resources->window = window;

    resources->instance = renderer_get_instance();

    resources->debug_callback_ext = renderer_get_debug_callback_ext(
        resources->instance
    );

    resources->surface = renderer_get_surface(
        resources->instance,
        window
    );

    resources->device_extensions[resources->device_extension_count] =
        calloc(1, strlen(VK_KHR_SWAPCHAIN_EXTENSION_NAME)+1);
    strcpy(
        resources->device_extensions[resources->device_extension_count++],
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    );

    resources->physical_device = renderer_get_physical_device(
        resources->instance,
        resources->surface,
        resources->device_extension_count,
        (const char**)resources->device_extensions
    );

    resources->device = renderer_get_device(
        resources->physical_device,
        resources->surface,
        NULL,
        resources->device_extension_count,
        (const char**)resources->device_extensions
    );

	resources->graphics_family_index = renderer_get_graphics_queue_family(
		resources->physical_device
    );
    resources->present_family_index = renderer_get_present_queue_family(
        resources->physical_device,
        resources->surface
    );

    vkGetDeviceQueue(
        resources->device,
        resources->graphics_family_index,
        0,
        &(resources->graphics_queue)
    );
    vkGetDeviceQueue(
        resources->device,
        resources->present_family_index,
        0,
        &(resources->present_queue)
    );

    resources->swapchain_image_format = renderer_get_swapchain_image_format(
		resources->physical_device,
		resources->surface
    );

    int window_width, window_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);
    resources->swapchain_extent = renderer_get_swapchain_extent(
        resources->physical_device,
        resources->surface,
        window_width,
        window_height
    );

    resources->swapchain = renderer_get_swapchain(
        resources->physical_device,
        resources->device,
        resources->surface,
        resources->swapchain_image_format,
        resources->swapchain_extent,
        VK_NULL_HANDLE
    );

    resources->image_count = renderer_get_swapchain_image_count(
        resources->device,
        resources->swapchain
    );

    resources->command_pool = renderer_get_command_pool(
        resources->physical_device,
        resources->device
    );

    resources->swapchain_buffers = malloc(
        resources->image_count * sizeof(*resources->swapchain_buffers)
    );
    renderer_create_swapchain_buffers(
        resources->device,
        resources->command_pool,
        resources->swapchain,
        resources->swapchain_image_format,
        resources->swapchain_buffers,
        resources->image_count
    );

    resources->depth_format = renderer_get_depth_format(
        resources->physical_device,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    resources->depth_image = renderer_get_image(
        resources->physical_device,
        resources->device,
        resources->swapchain_extent,
        resources->depth_format,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    renderer_set_depth_image_layout(
        resources->device,
        resources->graphics_queue,
        resources->command_pool,
        resources->depth_image.image,
        resources->depth_format
    );

    resources->descriptor_pool = renderer_get_descriptor_pool(
        resources->device
    );

    resources->descriptor_layout = renderer_get_descriptor_layout(
        resources->device
    );

    resources->uniform_buffer = renderer_get_buffer(
        resources->physical_device,
        resources->device,
        sizeof(mat4x4) * 3,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    resources->tex_image = renderer_load_texture(
        "assets/textures/plasma.png",
        resources->physical_device,
        resources->device,
        resources->graphics_queue,
        resources->command_pool
    );

    resources->descriptor_set = renderer_get_descriptor_set(
        resources->device,
        resources->descriptor_pool,
        &resources->descriptor_layout,
        1,
        &resources->uniform_buffer,
        &resources->tex_image
    );

    renderer_update_uniform_buffer(
        resources->device,
        resources->swapchain_extent,
        &resources->uniform_buffer,
        &resources->ubo,
        resources->camera,
        NULL
    );

	resources->render_pass = renderer_get_render_pass(
		resources->device,
		resources->swapchain_image_format.format,
        resources->depth_format
	);

    resources->pipeline_layout = renderer_get_pipeline_layout(
        resources->device,
        &resources->descriptor_layout,
        1,
        NULL,
        0
    );

    resources->graphics_pipeline = renderer_get_graphics_pipeline(
        resources->device,
        resources->swapchain_extent,
        resources->pipeline_layout,
        resources->render_pass,
        0
    );

    resources->framebuffers = malloc(
        sizeof(*resources->framebuffers) * resources->image_count);
	renderer_create_framebuffers(
		resources->device,
        resources->render_pass,
        resources->swapchain_extent,
        resources->swapchain_buffers,
        resources->depth_image.image_view,
        resources->framebuffers,
        resources->image_count
    );

	struct renderer_vertex vertices[] = {
        {.x=-0.5f,  .y=-0.5f,   .z=0.0f,    .u=1.0f,    .v=0.0f},
        {.x=0.5f,   .y=-0.5f,   .z=0.0f,    .u=0.0f,    .v=0.0f},
        {.x=0.5f,   .y=0.5f,    .z=0.0f,    .u=0.0f,    .v=1.0f},
        {.x=-0.5f,  .y=0.5f,    .z=0.0f,    .u=1.0f,    .v=1.0f},

        {.x=-0.5f,  .y=-0.5f,   .z=-.5f,    .u=1.0f,    .v=0.0f},
        {.x=0.5f,   .y=-0.5f,   .z=-.5f,    .u=0.0f,    .v=0.0f},
        {.x=0.5f,   .y=0.5f,    .z=-.5f,    .u=0.0f,    .v=1.0f},
        {.x=-0.5f,  .y=0.5f,    .z=-.5f,    .u=1.0f,    .v=1.0f}
    };
    resources->vbo = renderer_get_vertex_buffer(
        resources->physical_device,
        resources->device,
        resources->command_pool,
        resources->graphics_queue,
        vertices, 8
    );

	uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };
    resources->ibo = renderer_get_index_buffer(
        resources->physical_device,
        resources->device,
        resources->command_pool,
        resources->graphics_queue,
        indices, 12
    );
    resources->index_count = 12;

    renderer_record_draw_commands(
        resources->graphics_pipeline,
        resources->render_pass,
        resources->swapchain_extent,
        resources->framebuffers,
        resources->swapchain_buffers,
        resources->image_count,
        resources->vbo,
        resources->ibo,
        resources->index_count,
        resources->pipeline_layout,
        &resources->descriptor_set
    );

	resources->image_available = renderer_get_semaphore(resources->device);
	resources->render_finished = renderer_get_semaphore(resources->device);
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

    bool layer_found = true;
    for (uint32_t i = 0; i < enabled_layer_count; i++) {
        if (VALIDATION_ENABLED)
            break;

        layer_found = false;
        for (uint32_t j = 0; j < available_layer_count; j++) {
            if (!strcmp(enabled_layers[i], available_layers[j].layerName)) {
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

    for (uint32_t i = 0; i < glfw_extension_count; i++) {
        all_extensions[i] = calloc(1, strlen(glfw_extensions[i]) + 1);
        strcpy(all_extensions[i], glfw_extensions[i]);
    }

    for (uint32_t j = 0; j < my_extension_count; j++) {
        all_extensions[glfw_extension_count + j] = calloc(
            1, strlen(my_extensions[j]) + 1
        );
        strcpy(all_extensions[glfw_extension_count + j], my_extensions[j]);
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

    for (uint32_t i = 0; i < all_extension_count; i++)
        free(all_extensions[i]);

    free(all_extensions);
    free(available_layers);

    return instance_handle;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
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
        sprintf(message, "%s error, code %d: %s", pLayerPrefix, code, pMsg);
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        sprintf(message, "%s warning, code %d: %s", pLayerPrefix, code, pMsg);
    else
        return VK_FALSE;

    fprintf(stderr, "%s\n", message);

    free(message);

    return VK_FALSE;
}

VkDebugReportCallbackEXT renderer_get_debug_callback_ext(
        VkInstance instance)
{
    VkDebugReportCallbackEXT debug_callback_ext;

	PFN_vkCreateDebugReportCallbackEXT fp_create_debug_callback;
    fp_create_debug_callback =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                    instance,
                    "vkCreateDebugReportCallbackEXT"
            );
    assert(*fp_create_debug_callback);

    VkDebugReportCallbackCreateInfoEXT debug_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                VK_DEBUG_REPORT_WARNING_BIT_EXT,
        .pUserData = NULL,
        .pfnCallback = debug_callback,
    };

    VkResult result;
    result = fp_create_debug_callback(
        instance,
        &debug_info,
        NULL,
        &debug_callback_ext
    );
    assert(result == VK_SUCCESS);

    return debug_callback_ext;
}

void renderer_destroy_debug_callback_ext(
		VkInstance instance,
        VkDebugReportCallbackEXT debug_callback_ext)
{
    PFN_vkDestroyDebugReportCallbackEXT fp_destroy_debug_callback;
    fp_destroy_debug_callback =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
                    instance,
                    "vkDestroyDebugReportCallbackEXT"
            );
    assert(*fp_destroy_debug_callback);

    fp_destroy_debug_callback(
        instance,
        debug_callback_ext,
        NULL
    );
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
    for (uint32_t i = 0; i < required_extension_count; i++) {
        for (uint32_t j = 0; j < available_extension_count; j++) {
            if (strcmp(required_extensions[i],
                        available_extensions[j].extensionName)) {
                return false;
            }
        }
    }

    return true;
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

    for (uint32_t i = 0; i < physical_device_count; i++) {
        // Ensure required extensions are supported
        if (physical_device_extensions_supported(
                physical_devices[i],
                device_extension_count,
                device_extensions)) {
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

        VkBool32 wsi_support = VK_FALSE;
        VkBool32 graphics_bit = VK_FALSE;
        for (uint32_t j = 0; j < queue_family_count; j++) {
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

VkDevice renderer_get_device(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        VkPhysicalDeviceFeatures* required_features,
        uint32_t device_extension_count,
        const char** device_extensions)
{
    VkDevice device_handle;
    device_handle = VK_NULL_HANDLE;

    uint32_t graphics_family_index = renderer_get_graphics_queue_family(
        physical_device
    );
    uint32_t present_family_index = renderer_get_present_queue_family(
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

    for (uint32_t i = 0; i < device_queue_count; i++) {
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

uint32_t renderer_get_graphics_queue_family(
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

    bool graphics_queue_found = false;
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_family_properties[i].queueCount > 0 &&
                queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_index = i;
            graphics_queue_found = true;
            break;
        }
    }
    assert(graphics_queue_found);

    free(queue_family_properties);

    return graphics_queue_index;
}

uint32_t renderer_get_present_queue_family(
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

    VkBool32 wsi_support;
    bool present_queue_found = false;
    for (uint32_t i = 0; i < queue_family_count; i++) {
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

VkSurfaceFormatKHR renderer_get_swapchain_image_format(
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
    if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        image_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    // Limited selection of formats
    else
    {
        bool ideal_format_found = false;
        for (uint32_t i = 0; i < format_count; i++) {
            // Ideal format B8G8R8A8_UNORM, SRGB_NONLINEAR
            if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
                    formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                image_format = formats[i];
                ideal_format_found = true;
                break;
            }
        }

        if (!ideal_format_found) {
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

    if (surface_capabilities.currentExtent.width == UINT32_MAX) {
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
            desired_image_count > surface_capabilities.maxImageCount) {
        desired_image_count = surface_capabilities.maxImageCount;
    }

    // Queue family indices
    VkSharingMode sharing_mode;

    uint32_t queue_family_count;
    const uint32_t* queue_family_indices;

    uint32_t graphics_family_index = renderer_get_graphics_queue_family(
        physical_device
    );
    uint32_t present_family_index = renderer_get_present_queue_family(
        physical_device,
        surface
    );

    if (graphics_family_index != present_family_index) {
        sharing_mode = VK_SHARING_MODE_CONCURRENT;
        queue_family_count = 2;
        const uint32_t indices[] = {
            graphics_family_index,
            present_family_index
        };
        queue_family_indices = indices;
    } else {
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

    for (uint32_t i = 0; i < present_mode_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
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

    if (old_swapchain != VK_NULL_HANDLE) {
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

VkCommandPool renderer_get_command_pool(
        VkPhysicalDevice physical_device,
        VkDevice device)
{
    VkCommandPool command_pool_handle;
    command_pool_handle = VK_NULL_HANDLE;

    uint32_t graphics_family_index = renderer_get_graphics_queue_family(
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

void renderer_create_swapchain_buffers(
        VkDevice device,
        VkCommandPool command_pool,
        VkSwapchainKHR swapchain,
        VkSurfaceFormatKHR swapchain_image_format,
        struct renderer_swapchain_buffer* swapchain_buffers,
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

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
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

VkFormat renderer_get_depth_format(
        VkPhysicalDevice physical_device,
        VkImageTiling tiling,
        VkFormatFeatureFlags features)
{
    VkFormat format = VK_FORMAT_UNDEFINED;

    VkFormat formats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    uint32_t format_count = sizeof(formats)/sizeof(formats[0]);

    uint32_t i;
    for (i=0; i<format_count; i++)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(
            physical_device,
            formats[i],
            &properties
        );

        if ( (tiling == VK_IMAGE_TILING_LINEAR &&
             (properties.linearTilingFeatures & features) == features)
                ||
             (tiling == VK_IMAGE_TILING_OPTIMAL &&
             (properties.optimalTilingFeatures & features) == features) )
        {
            format = formats[i];
            break;
        }
    }

    return format;
}

void renderer_set_depth_image_layout(
        VkDevice device,
        VkQueue graphics_queue,
        VkCommandPool command_pool,
        VkImage depth_image,
        VkFormat depth_format)
{
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            depth_format == VK_FORMAT_D24_UNORM_S8_UINT) {
        aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    renderer_change_image_layout(
        device,
        graphics_queue,
        command_pool,
        depth_image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        0,
        aspect_mask
    );
}

VkDescriptorPool renderer_get_descriptor_pool(
        VkDevice device)
{
    VkDescriptorPool descriptor_pool_handle;
    descriptor_pool_handle = VK_NULL_HANDLE;

    VkDescriptorPoolSize ubo_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1
    };

    VkDescriptorPoolSize sampler_pool_size = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1
    };

    VkDescriptorPoolSize pool_sizes[] = {
        ubo_pool_size,
        sampler_pool_size
    };

    VkDescriptorPoolCreateInfo descriptor_pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 1,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes
    };

    VkResult result;
    result = vkCreateDescriptorPool(
        device,
        &descriptor_pool_info,
        NULL,
        &descriptor_pool_handle
    );
    assert(result == VK_SUCCESS);

    return descriptor_pool_handle;
}

VkDescriptorSetLayout renderer_get_descriptor_layout(
        VkDevice device)
{
    VkDescriptorSetLayout descriptor_layout_handle;
    descriptor_layout_handle = VK_NULL_HANDLE;

    VkDescriptorSetLayoutBinding ubo_layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding sampler_layout_binding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding layoutBindings[2] = {
        ubo_layout_binding,
        sampler_layout_binding
    };

    VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .bindingCount = 2,
        .pBindings = layoutBindings
    };

    VkResult result;
    result = vkCreateDescriptorSetLayout(
        device,
        &descriptor_layout_info,
        NULL,
        &descriptor_layout_handle
    );
    assert(result == VK_SUCCESS);

	return descriptor_layout_handle;
}

void renderer_update_uniform_buffer(
        VkDevice device,
        VkExtent2D swapchain_extent,
        struct renderer_buffer* uniform_buffer,
        struct renderer_ubo* ubo,
        struct camera camera,
        vec3 target)
{
    memset(ubo, 0, sizeof(*ubo));

    mat4x4_identity(ubo->model);
    mat4x4_rotate_all(ubo->model, 0.0f, 0.0f, 0.0f);

    vec3 eye = {camera.x, camera.y, camera.z};
    vec3 up = {0.0f, 0.0f, 1.0f};

    if (target) {
        mat4x4_look_at(ubo->view, eye, target, up);
    } else {
        vec3 center = {
            camera.x + cosf(camera.yaw),
            camera.y + sinf(camera.yaw),
            camera.pitch
        };
        mat4x4_look_at(ubo->view, eye, center, up);
    }

    float aspect = (float)swapchain_extent.width/swapchain_extent.height;

    // ~45 degree FOV
    mat4x4_perspective(ubo->projection, 0.78f, aspect, 0.1f, 100.0f);
    ubo->projection[1][1] *= -1;

    vkMapMemory(
        device,
        uniform_buffer->memory,
        0,
        uniform_buffer->size,
        0,
        &uniform_buffer->mapped
    );

    memcpy((float*)uniform_buffer->mapped + 16 * 0,
            ubo->model, sizeof(mat4x4));
    memcpy((float*)uniform_buffer->mapped + 16 * 1,
            ubo->view, sizeof(mat4x4));
    memcpy((float*)uniform_buffer->mapped + 16 * 2,
            ubo->projection, sizeof(mat4x4));

    vkUnmapMemory(device, uniform_buffer->memory);

    uniform_buffer->mapped = NULL;
}

VkDescriptorSet renderer_get_descriptor_set(
        VkDevice device,
        VkDescriptorPool descriptor_pool,
        VkDescriptorSetLayout* descriptor_layouts,
        uint32_t descriptor_count,
        struct renderer_buffer* uniform_buffer,
        struct renderer_image* tex_image)
{
    VkDescriptorSet descriptor_set_handle;
    descriptor_set_handle = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo descriptor_set_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = descriptor_count,
        .pSetLayouts = descriptor_layouts
    };

    VkResult result;
    result = vkAllocateDescriptorSets(
        device,
        &descriptor_set_info,
        &descriptor_set_handle
    );
    assert(result == VK_SUCCESS);

	VkDescriptorBufferInfo buffer_info = {
        .buffer = uniform_buffer->buffer,
        .offset = 0,
        .range = uniform_buffer->size
    };

	VkDescriptorImageInfo image_info = {
        .sampler = tex_image->sampler,
        .imageView = tex_image->image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet ubo_descriptor_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = NULL,
        .dstSet = descriptor_set_handle,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = NULL,
        .pBufferInfo = &buffer_info,
        .pTexelBufferView = NULL
    };

    VkWriteDescriptorSet sampler_descriptor_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = NULL,
        .dstSet = descriptor_set_handle,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
        .pBufferInfo = NULL,
        .pTexelBufferView = NULL
    };

	VkWriteDescriptorSet descriptor_writes[] = {
        ubo_descriptor_write,
        sampler_descriptor_write
    };

    vkUpdateDescriptorSets(device, 2, descriptor_writes, 0, NULL);

    return descriptor_set_handle;
}

VkRenderPass renderer_get_render_pass(
        VkDevice device,
        VkFormat image_format,
        VkFormat depth_format)
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

    VkAttachmentDescription depth_desc = {
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
    };

    VkAttachmentDescription attachments[] = {color_desc, depth_desc};

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depth_ref,
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
        .attachmentCount = 2,
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
        VkExtent2D swapchain_extent,
        VkPipelineLayout pipeline_layout,
        VkRenderPass render_pass,
        uint32_t subpass)
{
    VkShaderModule vert_shader_module;
    size_t vert_shader_size = renderer_get_file_size(
        "assets/shaders/vert.spv"
    );
    char* vert_shader_code = malloc(vert_shader_size);
    renderer_read_file_to_buffer(
        "assets/shaders/vert.spv",
        &vert_shader_code,
        vert_shader_size
    );
    vert_shader_module = renderer_get_shader_module(
        device,
        vert_shader_code,
        vert_shader_size
    );
    free(vert_shader_code);

    VkShaderModule frag_shader_module;
    size_t frag_shader_size = renderer_get_file_size(
        "assets/shaders/frag.spv"
    );
    char* frag_shader_code = malloc(frag_shader_size);
    renderer_read_file_to_buffer(
        "assets/shaders/frag.spv",
        &frag_shader_code,
        frag_shader_size
    );
    frag_shader_module = renderer_get_shader_module(
        device,
        frag_shader_code,
        frag_shader_size
    );
    free(frag_shader_code);

    VkShaderModule shader_modules[] = {
        vert_shader_module,
        frag_shader_module
    };
    VkShaderStageFlagBits shader_stage_flags[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };
    uint32_t shader_stage_count = 2;

    VkPipelineShaderStageCreateInfo* shader_infos;
    shader_infos = malloc(shader_stage_count * sizeof(*shader_infos));

    for (uint32_t i=0; i<shader_stage_count; i++) {
        shader_infos[i].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_infos[i].pNext = NULL;
        shader_infos[i].flags = 0;
        shader_infos[i].stage = shader_stage_flags[i];
        shader_infos[i].module = shader_modules[i];
        shader_infos[i].pName = "main";
        shader_infos[i].pSpecializationInfo = NULL;
    }

    VkVertexInputBindingDescription binding_description = {
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
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attribute_descriptions
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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

    vkDestroyShaderModule(device, vert_shader_module, NULL);
    vkDestroyShaderModule(device, frag_shader_module, NULL);

    return graphics_pipeline_handle;
}

void renderer_create_framebuffers(
        VkDevice device,
        VkRenderPass render_pass,
        VkExtent2D swapchain_extent,
        struct renderer_swapchain_buffer* swapchain_buffers,
        VkImageView depth_image_view,
        VkFramebuffer* framebuffers,
        uint32_t swapchain_image_count)
{
    VkImageView attachments[2];
    attachments[1] = depth_image_view;

    VkFramebufferCreateInfo framebuffer_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .renderPass = render_pass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = swapchain_extent.width,
        .height = swapchain_extent.height,
        .layers = 1
    };

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        attachments[0] = swapchain_buffers[i].image_view;

        VkResult result;
        result = vkCreateFramebuffer(
            device,
            &framebuffer_info,
            NULL,
            &framebuffers[i]
        );
        assert(result == VK_SUCCESS);
    }
}

struct renderer_buffer renderer_get_vertex_buffer(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkCommandPool command_pool,
        VkQueue queue,
        struct renderer_vertex* vertices,
        uint32_t vertex_count)
{
    struct renderer_buffer vbo;
    struct renderer_buffer staging_vbo;

    VkDeviceSize mem_size = sizeof(*vertices) * vertex_count;

    staging_vbo = renderer_get_buffer(
        physical_device,
        device,
        mem_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    vkMapMemory(device, staging_vbo.memory, 0, mem_size, 0, &vbo.mapped);
    memcpy(vbo.mapped, vertices, (size_t)mem_size);
    vkUnmapMemory(device, staging_vbo.memory);

    vbo = renderer_get_buffer(
        physical_device,
        device,
        mem_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    renderer_copy_buffer_to_buffer(
        device,
        command_pool,
        queue,
        staging_vbo.buffer,
        vbo.buffer,
        mem_size
    );

    vkDestroyBuffer(device, staging_vbo.buffer, NULL);
    vkFreeMemory(device, staging_vbo.memory, NULL);

    return vbo;
}

struct renderer_buffer renderer_get_index_buffer(
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkCommandPool command_pool,
        VkQueue queue,
        uint32_t* indices,
        uint32_t index_count)
{
    struct renderer_buffer ibo;
    struct renderer_buffer staging_ibo;

    VkDeviceSize mem_size = sizeof(*indices) * index_count;

    staging_ibo = renderer_get_buffer(
        physical_device,
        device,
        mem_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    vkMapMemory(device, staging_ibo.memory, 0, mem_size, 0, &ibo.mapped);
    memcpy(ibo.mapped, indices, (size_t)mem_size);
    vkUnmapMemory(device, staging_ibo.memory);

    ibo = renderer_get_buffer(
        physical_device,
        device,
        mem_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    renderer_copy_buffer_to_buffer(
        device,
        command_pool,
        queue,
        staging_ibo.buffer,
        ibo.buffer,
        mem_size
    );

    vkDestroyBuffer(device, staging_ibo.buffer, NULL);
    vkFreeMemory(device, staging_ibo.memory, NULL);

    return ibo;
}

void renderer_record_draw_commands(
        VkPipeline pipeline,
        //VkPipelineLayout pipeline_layout,
        VkRenderPass render_pass,
        VkExtent2D swapchain_extent,
        VkFramebuffer* framebuffers,
        struct renderer_swapchain_buffer* swapchain_buffers,
        uint32_t swapchain_image_count,
        struct renderer_buffer vbo,
        struct renderer_buffer ibo,
		uint32_t index_count,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet* descriptor_sets)
        //struct renderer_mesh* mesh)
{
    VkCommandBufferBeginInfo cmd_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL
    };

    VkClearValue clear_values[] = {
        {.color.float32 = {0.0f, 0.0f, 0.0f, 1.0f}},
        {.depthStencil = {1.0f, 0}}
    };

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = render_pass,
        .renderArea.offset = {0,0},
        .renderArea.extent = {swapchain_extent.width, swapchain_extent.height},
        .clearValueCount = 2,
        .pClearValues = clear_values,
    };

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        VkResult result;
        result = vkBeginCommandBuffer(
            swapchain_buffers[i].cmd,
            &cmd_begin_info
        );
        assert(result == VK_SUCCESS);

        render_pass_info.framebuffer = framebuffers[i];
        vkCmdBeginRenderPass(
            swapchain_buffers[i].cmd,
            &render_pass_info,
            VK_SUBPASS_CONTENTS_INLINE
        );

        VkViewport viewport = {
            .x = 0,
            .y = 0,
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(swapchain_buffers[i].cmd, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0,0},
            .extent = {swapchain_extent.width, swapchain_extent.height}
        };
        vkCmdSetScissor(swapchain_buffers[i].cmd, 0, 1, &scissor);

        vkCmdBindPipeline(
            swapchain_buffers[i].cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline
        );

        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(
            swapchain_buffers[i].cmd,
            0,
            1,
            &vbo.buffer,
            offsets
        );

        vkCmdBindIndexBuffer(
            swapchain_buffers[i].cmd,
            ibo.buffer,
            0,
            VK_INDEX_TYPE_UINT32
        );

        vkCmdBindDescriptorSets(
            swapchain_buffers[i].cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0,
            1,
            descriptor_sets,
            //&mesh->descriptor_set,
            0,
            NULL
        );

        vkCmdDrawIndexed(
            swapchain_buffers[i].cmd,
            index_count,
            1,
            0,
            0,
            0
        );

        vkCmdEndRenderPass(swapchain_buffers[i].cmd);

        result = vkEndCommandBuffer(swapchain_buffers[i].cmd);
        assert(result == VK_SUCCESS);
    }
}

VkSemaphore renderer_get_semaphore(
        VkDevice device)
{
    VkSemaphore semaphore_handle;

    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };

    VkResult result;
    result = vkCreateSemaphore(
        device,
        &semaphore_info,
        NULL,
        &semaphore_handle
    );
    assert(result == VK_SUCCESS);

    return semaphore_handle;
}

void drawFrame(struct renderer_resources* resources)
{
    renderer_update_uniform_buffer(
        resources->device,
        resources->swapchain_extent,
        &resources->uniform_buffer,
        &resources->ubo,
        resources->camera,
        NULL
    );

    uint32_t image_index;

    VkResult result;
    result = vkAcquireNextImageKHR(
        resources->device,
        resources->swapchain,
        UINT64_MAX, // Wait for next image indefinitely (ns)
        resources->image_available,
        VK_NULL_HANDLE,
        &image_index
    );
    assert(result == VK_SUCCESS);

    VkSemaphore wait_semaphores[] = {resources->image_available};
    VkSemaphore signal_semaphores[] = {resources->render_finished};

    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &resources->swapchain_buffers[image_index].cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores
    };

    result = vkQueueSubmit(
        resources->graphics_queue,
        1,
        &submit_info,
        VK_NULL_HANDLE
    );
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Error while submitting queue.\n");
        exit(-1);
    }

    VkSwapchainKHR swapchains[] = {resources->swapchain};

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index,
        .pResults = NULL
    };

    vkQueuePresentKHR(resources->present_queue, &present_info);

    vkDeviceWaitIdle(resources->device);
}

void renderer_resize(
        struct renderer_resources* resources,
        int width,
        int height)
{
    vkDeviceWaitIdle(resources->device);

    for (uint32_t i = 0; i < resources->image_count; i++) {
        vkDestroyFramebuffer(
            resources->device,
            resources->framebuffers[i],
            NULL
        );
    }

    vkDestroyPipeline(
        resources->device,
        resources->graphics_pipeline,
        NULL
    );

    vkDestroyPipelineLayout(
        resources->device,
        resources->pipeline_layout,
        NULL
    );

    vkDestroyRenderPass(resources->device, resources->render_pass, NULL);

    vkDestroyImage(resources->device, resources->depth_image.image, NULL);
    vkDestroyImageView(
        resources->device,
        resources->depth_image.image_view,
        NULL
    );
    vkFreeMemory(resources->device, resources->depth_image.memory, NULL);

    for (uint32_t i = 0; i < resources->image_count; i++) {
        vkDestroyImageView(
            resources->device,
            resources->swapchain_buffers[i].image_view,
            NULL
        );
        vkFreeCommandBuffers(
            resources->device,
            resources->command_pool,
            1,
            &resources->swapchain_buffers[i].cmd
        );
    }

    resources->swapchain_extent = renderer_get_swapchain_extent(
        resources->physical_device,
        resources->surface,
        width,
        height
    );

    resources->swapchain = renderer_get_swapchain(
        resources->physical_device,
        resources->device,
        resources->surface,
        resources->swapchain_image_format,
        resources->swapchain_extent,
        resources->swapchain
    );

    resources->image_count = renderer_get_swapchain_image_count(
        resources->device,
        resources->swapchain
    );

    renderer_create_swapchain_buffers(
        resources->device,
        resources->command_pool,
        resources->swapchain,
        resources->swapchain_image_format,
        resources->swapchain_buffers,
        resources->image_count
    );

    resources->depth_image = renderer_get_image(
        resources->physical_device,
        resources->device,
        resources->swapchain_extent,
        resources->depth_format,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    renderer_set_depth_image_layout(
        resources->device,
        resources->graphics_queue,
        resources->command_pool,
        resources->depth_image.image,
        resources->depth_format
    );

	resources->render_pass = renderer_get_render_pass(
		resources->device,
		resources->swapchain_image_format.format,
        resources->depth_format
	);

    resources->pipeline_layout = renderer_get_pipeline_layout(
        resources->device,
        &resources->descriptor_layout,
        1,
        NULL,
        0
    );

    resources->graphics_pipeline = renderer_get_graphics_pipeline(
        resources->device,
        resources->swapchain_extent,
        resources->pipeline_layout,
        resources->render_pass,
        0
    );

	renderer_create_framebuffers(
		resources->device,
        resources->render_pass,
        resources->swapchain_extent,
        resources->swapchain_buffers,
        resources->depth_image.image_view,
        resources->framebuffers,
        resources->image_count
    );

    renderer_record_draw_commands(
        resources->graphics_pipeline,
        resources->render_pass,
        resources->swapchain_extent,
        resources->framebuffers,
        resources->swapchain_buffers,
        resources->image_count,
        resources->vbo,
        resources->ibo,
        resources->index_count,
        resources->pipeline_layout,
        &resources->descriptor_set
    );
}

void renderer_destroy_resources(
        struct renderer_resources* resources)
{
    vkDestroyImage(resources->device, resources->tex_image.image, NULL);
    vkDestroyImageView(
        resources->device,
        resources->tex_image.image_view,
        NULL
    );
    vkDestroySampler(resources->device, resources->tex_image.sampler, NULL);
    vkFreeMemory(resources->device, resources->tex_image.memory, NULL);

    vkDestroyBuffer(resources->device, resources->vbo.buffer, NULL);
    vkFreeMemory(resources->device, resources->vbo.memory, NULL);
    vkDestroyBuffer(resources->device, resources->ibo.buffer, NULL);
    vkFreeMemory(resources->device, resources->ibo.memory, NULL);

    vkDestroySemaphore(resources->device, resources->image_available, NULL);
    vkDestroySemaphore(resources->device, resources->render_finished, NULL);

    for (uint32_t i = 0; i < resources->image_count; i++) {
        vkDestroyFramebuffer(
            resources->device,
            resources->framebuffers[i],
            NULL
        );
    }

    vkDestroyPipeline(
        resources->device,
        resources->graphics_pipeline,
        NULL
    );

    vkDestroyPipelineLayout(
        resources->device,
        resources->pipeline_layout,
        NULL
    );

    vkDestroyRenderPass(resources->device, resources->render_pass, NULL);

    vkFreeMemory(resources->device, resources->uniform_buffer.memory, NULL);
    vkDestroyBuffer(resources->device, resources->uniform_buffer.buffer, NULL);

    vkDestroyDescriptorSetLayout(
        resources->device,
        resources->descriptor_layout,
        NULL
    );

    vkDestroyDescriptorPool(
        resources->device,
        resources->descriptor_pool,
        NULL
    );

    vkDestroyImage(resources->device, resources->depth_image.image, NULL);
    vkDestroyImageView(
        resources->device,
        resources->depth_image.image_view,
        NULL
    );
    vkFreeMemory(resources->device, resources->depth_image.memory, NULL);

    for (uint32_t i = 0; i < resources->image_count; i++) {
        vkDestroyImageView(
            resources->device,
            resources->swapchain_buffers[i].image_view,
            NULL
        );
        vkFreeCommandBuffers(
            resources->device,
            resources->command_pool,
            1,
            &resources->swapchain_buffers[i].cmd
        );
    }
    free(resources->swapchain_buffers);

    vkDestroyCommandPool(resources->device, resources->command_pool, NULL);

    vkDestroySwapchainKHR(resources->device, resources->swapchain, NULL);

    vkDestroyDevice(resources->device, NULL);

    vkDestroySurfaceKHR(resources->instance, resources->surface, NULL);

    for (uint32_t i = 0; i < resources->surface_extension_count; i++)
        free(resources->surface_extensions[i]);

    for (uint32_t i = 0; i < resources->device_extension_count; i++)
        free(resources->device_extensions[i]);

    renderer_destroy_debug_callback_ext(
        resources->instance,
        resources->debug_callback_ext
    );

    vkDestroyInstance(resources->instance, NULL);
}
