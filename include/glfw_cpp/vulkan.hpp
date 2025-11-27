#ifndef GLFW_CPP_VULKAN_HPP
#define GLFW_CPP_VULKAN_HPP

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

    bool                     vulkan_supported();
    std::vector<const char*> get_required_instance_extensions();

#if defined(VK_VERSION_1_0)
    VkProc get_instance_proc_address(VkInstance instance, const char* proc_name);

    bool get_physical_device_presentation_support(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queue_family
    );

    VkResult create_surface(
        const Window&                window,
        VkInstance                   instance,
        const VkAllocationCallbacks* allocator,
        VkSurfaceKHR*                surface
    );

#endif    // VK_VERSION_1_0

#ifdef VULKAN_HPP
    inline VkProc get_instance_proc_address(const ::vk::Instance& instance, const char* proc_name)
    {
        return get_instance_proc_address(static_cast<VkInstance>(instance), proc_name);
    }

    inline bool get_physical_device_presentation_support(
        const ::vk::Instance&       instance,
        const ::vk::PhysicalDevice& device,
        uint32_t                    queue_family
    )
    {
        return get_physical_device_presentation_support(
            static_cast<VkInstance>(instance), static_cast<VkPhysicalDevice>(device), queue_family
        );
    }

    inline ::vk::ResultValue<::vk::SurfaceKHR> create_surface(
        const Window&                             window,
        const ::vk::Instance&                     instance,
        ::vk::Optional<::vk::AllocationCallbacks> allocator = nullptr
    )
    {
        auto surface    = VkSurfaceKHR{};
        auto result     = VkResult{};
        auto c_instance = static_cast<VkInstance>(instance);

        if (allocator) {
            auto c_alloc = static_cast<VkAllocationCallbacks>(*allocator);
            result       = create_surface(window, c_instance, &c_alloc, &surface);
        } else {
            result = create_surface(window, c_instance, nullptr, &surface);
        }

        return { ::vk::Result(result), ::vk::SurfaceKHR(surface) };
    }

#endif    // VULKAN_HPP
}

#endif /* end of include guard: GLFW_CPP_VULKAN_HPP */
