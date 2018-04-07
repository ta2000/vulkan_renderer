#ifndef RENDERER_H_
#define RENDERER_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "queue.h"

#include <stdbool.h>

#define VALIDATION_ENABLED 1

#define APP_NAME "Game"
#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// TODO:
// - Pool for queues?
// - Device features
// - Save aspect ratio everytime window is resized ( instead of every time uniform buffer
// updated)
// - Use one command buffer for all transfer command stuff created at beginning
// - Update uniform buffer on resize while paused
// - Remove glfwGetWindowSize for swapchain image size

struct camera
{
    float x, y, z;
    float pitch, yaw;
};

struct renderer_vertex
{
    float x, y, z;
    float u, v;
};

struct renderer_swapchain_buffer
{
    VkImage image;
    VkImageView image_view;
    VkCommandBuffer cmd;
};

struct renderer_resources
{
    GLFWwindow* window;

    struct camera camera;

    struct renderer_mesh* meshes;
    struct queue mesh_draw_queue;

    VkInstance instance;

    char* surface_extensions[64];
    uint32_t surface_extension_count;
    char* device_extensions[64];
    uint32_t device_extension_count;
    VkDebugReportCallbackEXT debug_callback_ext;
    PFN_vkCreateDebugReportCallbackEXT create_debug_callback;
    PFN_vkDestroyDebugReportCallbackEXT destroy_debug_callback;

    VkSurfaceKHR surface;

    VkQueue graphics_queue;
    VkQueue present_queue;
    int graphics_family_index;
    int present_family_index;

    VkPhysicalDevice physical_device;
    VkDevice device;

    VkSwapchainKHR swapchain;
    uint32_t image_count;
    struct renderer_swapchain_buffer* swapchain_buffers;
    VkSurfaceFormatKHR swapchain_image_format;
    VkExtent2D swapchain_extent;

    VkFormat depth_format;
    struct renderer_image depth_image;

    VkCommandPool command_pool;

    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_layout;
    VkDescriptorSet descriptor_set;

    struct renderer_image tex_image;

    struct renderer_buffer dynamic_uniform_buffer;
    struct renderer_buffer view_projection_uniform_buffer;
    mat4x4 model_matrix;
    mat4x4 view_matrix;
    mat4x4 projection_matrix;

    VkRenderPass render_pass;

    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    VkFramebuffer* framebuffers;

    struct renderer_buffer vbo;
    struct renderer_buffer ibo;
    uint32_t index_count;

    VkSemaphore image_available;
    VkSemaphore render_finished;
};

void renderer_initialize_resources(
    struct renderer_resources* resources,
    GLFWwindow* window
);

VkInstance renderer_get_instance();

VkDebugReportCallbackEXT renderer_get_debug_callback_ext(
    VkInstance instance
);

void renderer_destroy_debug_callback_ext(
    VkInstance instance,
    VkDebugReportCallbackEXT debug_callback_ext
);

VkSurfaceKHR renderer_get_surface(
	VkInstance instance,
	GLFWwindow* window
);

bool physical_device_extensions_supported(
    VkPhysicalDevice physical_device,
    uint32_t required_extension_count,
    const char** required_extensions
);

VkPhysicalDevice renderer_get_physical_device(
	VkInstance instance,
	VkSurfaceKHR surface,
	uint32_t device_extension_count,
	const char** device_extensions
);

VkDevice renderer_get_device(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    VkPhysicalDeviceFeatures* required_features,
    uint32_t device_extension_count,
    const char** device_extensions
);

uint32_t renderer_get_graphics_queue_family(
	VkPhysicalDevice physical_device
);

uint32_t renderer_get_present_queue_family(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);

VkSurfaceFormatKHR renderer_get_swapchain_image_format(
	VkPhysicalDevice physical_device,
	VkSurfaceKHR surface
);

VkExtent2D renderer_get_swapchain_extent(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    uint32_t window_width,
    uint32_t window_height
);

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

VkCommandPool renderer_get_command_pool(
    VkPhysicalDevice physical_device,
    VkDevice device
);

void renderer_create_swapchain_buffers(
    VkDevice device,
    VkCommandPool command_pool,
    VkSwapchainKHR swapchain,
    VkSurfaceFormatKHR swapchain_image_format,
    struct renderer_swapchain_buffer* swapchain_buffers,
    uint32_t swapchain_image_count
);

VkFormat renderer_get_depth_format(
    VkPhysicalDevice physical_device,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
);

void renderer_set_depth_image_layout(
    VkDevice device,
    VkQueue graphics_queue,
    VkCommandPool command_pool,
    VkImage depth_image,
    VkFormat depth_format
);

VkDescriptorPool renderer_get_descriptor_pool(
    VkDevice device
);

VkDescriptorSetLayout renderer_get_descriptor_layout(
    VkDevice device
);

VkDescriptorSet renderer_get_descriptor_set(
    VkDevice device,
    VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout* descriptor_layouts,
    uint32_t descriptor_count,
    struct renderer_buffer* uniform_buffer_instance,
    struct renderer_buffer* uniform_buffer_view,
    struct renderer_image* tex_image
);

void renderer_update_dynamic_uniform_buffer(
    VkDevice device,
    struct renderer_buffer* uniform_buffer_instance,
    mat4x4 model_matrix
);

void renderer_update_view_projection_uniform_buffer(
    VkExtent2D swapchain_extent,
    struct renderer_buffer* uniform_buffer,
    mat4x4 view_matrix,
    mat4x4 projection_matrix,
    struct camera camera,
    vec3 target
);

VkRenderPass renderer_get_render_pass(
	VkDevice device,
	VkFormat image_format,
	VkFormat depth_format
);

VkPipelineLayout renderer_get_pipeline_layout(
    VkDevice device,
    VkDescriptorSetLayout* descriptor_layouts,
    uint32_t descriptor_layout_count,
    VkPushConstantRange* push_constant_ranges,
    uint32_t push_constant_range_count
);

VkShaderModule renderer_get_shader_module(
    VkDevice device,
    char* code,
    size_t code_size
);

VkPipeline renderer_get_graphics_pipeline(
    VkDevice device,
    VkExtent2D swapchain_extent,
    VkPipelineLayout pipeline_layout,
    VkRenderPass render_pass,
    uint32_t subpass
);

void renderer_create_framebuffers(
	VkDevice device,
	VkRenderPass render_pass,
	VkExtent2D swapchain_extent,
	struct renderer_swapchain_buffer* swapchain_buffers,
	VkImageView depth_image_view,
	VkFramebuffer* framebuffers,
	uint32_t swapchain_image_count
);

struct renderer_buffer renderer_get_vertex_buffer(
	VkPhysicalDevice physical_device,
	VkDevice device,
	VkCommandPool command_pool,
	VkQueue queue,
	struct renderer_vertex* vertices,
	uint32_t vertex_count
);

struct renderer_buffer renderer_get_index_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    uint32_t* indices,
    uint32_t index_count
);

void renderer_record_draw_commands(
    VkPipeline pipeline,
    VkRenderPass render_pass,
    VkExtent2D swapchain_extent,
    VkFramebuffer framebuffer,
    struct renderer_swapchain_buffer swapchain_buffer,
    struct queue* mesh_draw_queue,
    VkPipelineLayout pipeline_layout,
    VkDescriptorSet* descriptor_set
);

VkSemaphore renderer_get_semaphore(
    VkDevice device
);

void renderer_draw_frame(
    struct renderer_resources* resources
);

void renderer_resize(
    struct renderer_resources* resources,
    int width,
    int height
);

void renderer_destroy_resources(
    struct renderer_resources* resources
);

void renderer_generate_meshes(
    struct renderer_resources* resources,
    const char** models,
    const uint32_t model_count
);

void renderer_destroy_meshes(
    struct renderer_resources* resources
);

void renderer_get_model_vertex_count(
    const char* src,
    uint32_t* vertex_count,
    uint32_t* index_count
);

void renderer_load_model(
    const char* src,
    struct renderer_vertex* vertices,
    uint32_t* indices
);

#endif
