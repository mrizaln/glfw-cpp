// vk tricks
// ---------
#include <cstdint>

#ifndef VK_USE_64_BIT_PTR_DEFINES
    #if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__))                 \
        || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__)                    \
        || defined(__powerpc64__) || (defined(__riscv) && __riscv_xlen == 64)
        #define VK_USE_64_BIT_PTR_DEFINES 1
    #else
        #define VK_USE_64_BIT_PTR_DEFINES 0
    #endif
#endif

#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#ifndef VK_DEFINE_NON_DISPATCHABLE_HANDLE
    #if (VK_USE_64_BIT_PTR_DEFINES == 1)
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object;
    #else
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
    #endif
#endif

// is this extern C needed? should I simplify the typedef by using using instead?
extern "C" {
// vk types definition for use in this unit only

typedef enum VkResult
{
} VkResult;

typedef struct VkExtensionProperties
{
    char     extensionName[256];
    uint32_t specVersion;
} VkExtensionProperties;

VK_DEFINE_HANDLE(VkInstance);
VK_DEFINE_HANDLE(VkPhysicalDevice);

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR);

typedef struct VkAllocationCallbacks VkAllocationCallbacks;

typedef void (*PFN_vkVoidFunction)(void);

typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr)(VkInstance, const char*);
typedef VkResult (*PFN_vkEnumerateInstanceExtensionProperties)(const char*, uint32_t*, VkExtensionProperties*);
}

// tricks <glfw_cpp/vulkan.hpp> and GLFW into thinking that it has Vulkan included so that I can use the
// functions defined in GLFW as long as I have the required types defined
#define VK_VERSION_1_0
// ---------

#include "glfw_cpp/window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "util.hpp"

namespace glfw_cpp::vk
{
    using Proc = void (*)(void);

    void init_vulkan_loader(PFN_vkGetInstanceProcAddr loader)
    {
        glfwInitVulkanLoader(loader);
    }

    bool vulkan_supported()
    {
        return glfwVulkanSupported();
    }

    std::span<const char*> get_required_instance_extensions()
    {
        auto count             = uint32_t{};
        auto p_extension_names = glfwGetRequiredInstanceExtensions(&count);
        return { p_extension_names, count };
    }

    Proc get_instance_proc_address(VkInstance instance, const char* proc_name)
    {
        auto addr = glfwGetInstanceProcAddress(instance, proc_name);
        util::check_glfw_error();
        return addr;
    }

    Proc get_instance_proc_address_noexcept(VkInstance instance, const char* proc_name) noexcept
    {
        return glfwGetInstanceProcAddress(instance, proc_name);
    }

    bool get_physical_device_presentation_support(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queue_family
    )
    {
        return glfwGetPhysicalDevicePresentationSupport(instance, device, queue_family);
    }

    VkResult create_surface(
        const Window&                window,
        VkInstance                   instance,
        const VkAllocationCallbacks* allocator,
        VkSurfaceKHR*                surface
    )
    {
        return glfwCreateWindowSurface(instance, window.handle(), allocator, surface);
    }
}
