#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "glfw_cpp/vulkan.hpp"
#include "glfw_cpp/window.hpp"

#include <vector>

namespace glfw_cpp::vk
{
    bool vulkanSupported()
    {
        return glfwVulkanSupported();
    }

    std::vector<const char*> getRequiredInstanceExtensions()
    {
        uint32_t count;
        auto     pExtensionNames = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char*> extensionNames;
        extensionNames.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            extensionNames.push_back(pExtensionNames[i]);
        }
        return extensionNames;
    }

    VkResult createSurface(
        const Window&                window,
        VkInstance                   instance,
        const VkAllocationCallbacks* allocator,
        VkSurfaceKHR*                surface
    )
    {
        return glfwCreateWindowSurface(instance, window.handle(), allocator, surface);
    }

    bool getPhysicalDevicePresentationSupport(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queueFamily
    )
    {
        return glfwGetPhysicalDevicePresentationSupport(instance, device, queueFamily);
    }

    VkProc getInstanceProcAddress(VkInstance instance, const char* procName)
    {
        return glfwGetInstanceProcAddress(instance, procName);
    }
}
