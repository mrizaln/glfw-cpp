#ifndef VULKAN_HPP_VGY7T48UE
#define VULKAN_HPP_VGY7T48UE

#if not defined(GLFW_CPP_VULKAN_SUPPORT)
#    error "Vulkan support is not enabled"
#endif

#if not defined(VK_VERSION_1_0) and not defined(VULKAN_HPP)
#    error "Include Vulkan headers (vulkan.h or vulkan.hpp) before including this header"
#endif

#include "glfw_cpp/window.hpp"

namespace glfw_cpp::vk
{
    using VkProc = void (*)(void);

    bool                     vulkanSupported();
    std::vector<const char*> getRequiredInstanceExtensions();

#if defined(VK_VERSION_1_0)
    using VkLoaderFun = glfw_cpp::vk::VkProc (*)(VkInstance instance, const char* pName);

    /* Sets the Vulkan loader function that GLFW use for all vulkan related entry point queries.
     * Normally, you don't need to call this function, GLFW will try to load the Vulkan loader
     * automatically. But if you want to use a custom loader, you can call this function before
     * creating the first Vulkan instance.
     */
    void initVulkanLoader(VkLoaderFun loader);

    VkProc getInstanceProcAddress(VkInstance instance, const char* procName);

    bool getPhysicalDevicePresentationSupport(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queueFamily
    );

    VkResult createSurface(
        const Window&                window,
        VkInstance                   instance,
        const VkAllocationCallbacks* allocator,
        VkSurfaceKHR*                surface
    );

#endif    // VK_VERSION_1_0

#ifdef VULKAN_HPP
    inline VkProc getInstanceProcAddress(const ::vk::Instance& instance, const char* procName)
    {
        return getInstanceProcAddress(static_cast<VkInstance>(instance), procName);
    }

    inline bool getPhysicalDevicePresentationSupport(
        const ::vk::Instance&       instance,
        const ::vk::PhysicalDevice& device,
        uint32_t                    queueFamily
    )
    {
        return getPhysicalDevicePresentationSupport(
            static_cast<VkInstance>(instance), static_cast<VkPhysicalDevice>(device), queueFamily
        );
    }

    inline ::vk::ResultValue<::vk::SurfaceKHR> createSurface(
        const Window&                             window,
        const ::vk::Instance&                     instance,
        ::vk::Optional<::vk::AllocationCallbacks> allocator = nullptr
    )
    {
        VkSurfaceKHR surface;
        VkResult     result;
        auto         cInstance = static_cast<VkInstance>(instance);

        if (allocator) {
            auto cAlloc = static_cast<VkAllocationCallbacks>(*allocator);
            result      = createSurface(window, cInstance, &cAlloc, &surface);
        } else {
            result = createSurface(window, cInstance, nullptr, &surface);
        }

        return { ::vk::Result(result), ::vk::SurfaceKHR(surface) };
    }

#endif    // VULKAN_HPP
}

#endif /* end of include guard: VULKAN_HPP_VGY7T48UE */
