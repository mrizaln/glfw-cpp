#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "glfw_cpp/vulkan.hpp"
#include "glfw_cpp/window.hpp"

#include <vector>

namespace glfw_cpp::vk
{
    bool vulkan_supported()
    {
        return glfwVulkanSupported();
    }

    std::vector<const char*> get_required_instance_extensions()
    {
        auto count             = uint32_t{};
        auto p_extension_names = glfwGetRequiredInstanceExtensions(&count);

        auto extension_names = std::vector<const char*>{};
        extension_names.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            extension_names.push_back(p_extension_names[i]);
        }
        return extension_names;
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

    bool get_physical_device_presentation_support(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queue_family
    )
    {
        return glfwGetPhysicalDevicePresentationSupport(instance, device, queue_family);
    }

    VkProc get_instance_proc_address(VkInstance instance, const char* proc_name)
    {
        return glfwGetInstanceProcAddress(instance, proc_name);
    }
}
