#ifndef GLFW_CPP_VULKAN_HPP
#define GLFW_CPP_VULKAN_HPP

#if defined(GLFW_CPP_INCLUDE_VULKAN_H)
    #include <vulkan/vulkan.h>
#endif
#if defined(GLFW_CPP_INCLUDE_VULKAN_HPP)
    #include <vulkan/vulkan.hpp>
#endif

#include <span>

namespace glfw_cpp
{
    class Window;
}

namespace glfw_cpp::vk
{
    using Proc = void (*)(void);

#if defined(VK_VERSION_1_0)
    /**
     * @brief set the `vkGetInstanceProcAddr` function that GLFW will use for all Vulkan related entry point
     * queries.
     *
     * @param loader The address of the function to use, or `nullptr`.
     *
     * @thread_safety This function must be called from the main thread.
     *
     * @warning This function must be called before `init()`.
     *
     * If set to NULL, GLFW will try to load the Vulkan loader dynamically by its standard name and get this
     * function from there. This is the default behavior.
     *
     * This feature is mostly useful on macOS, if your copy of the Vulkan loader is in a location where GLFW
     * cannot find it through dynamic loading, or if you are still using the static library version of the
     * loader.
     */
    void init_vulkan_loader(PFN_vkGetInstanceProcAddr loader);
#endif

    /**
     * @brief Check whether the Vulkan loader and any minimally functional ICD have been found.
     *
     * @return True if Vulkan is minimally available, false otherwise.
     *
     * @thread_safety This function may be called from any thread.
     *
     * @throw error::NotInitialized If glfw is not initialized yet.
     */
    bool vulkan_supported();

    /**
     * @brief Return an span of names of Vulkan instance extensions required by GLFW for creating Vulkan
     * surfaces for GLFW windows
     *
     * @thread_safety This function may be called from any thread.
     */
    std::span<const char*> get_required_instance_extensions();

#if defined(VK_VERSION_1_0)
    /**
     * @brief Return the address of the specified vulkan core or extension function for the specified
     * instance.
     *
     * @param instance The vulkan instance to query or `nullptr` to retrieve functions related to instance
     * creation.
     * @param proc_name The ASCII encoded name of the function.
     * @return The address of the function or `nullptr` if an error occurred.
     *
     * @thread_safety This function may be called from any thread.
     *
     * @throw error::NotInitialized If GLFW is not initialized yet.
     * @throw error::ApiUnavailable If Vulkan API is not available.
     *
     * If you need to call this function from C (which can't handle exception) you may want to use `noexcept`
     * version of this function: `get_instace_proc_address_noexcept()`.
     */
    Proc get_instance_proc_address(VkInstance instance, const char* proc_name);

    /**
     * @brief Return the address of the specified vulkan core or extension function for the specified
     * instance.
     *
     * @param instance The vulkan instance to query or `nullptr` to retrieve functions related to instance
     * creation.
     * @param proc_name The ASCII encoded name of the function.
     * @return The address of the function or `nullptr` if an error occurred.
     *
     * @thread_safety This function may be called from any thread.
     *
     * The error will be logged but this function won't throw to preserve the behavior from the underlying
     * function. The possible errors are:
     * - `ErrorCode::NotInitialized`, and
     * - `ErrorCode::ApiUnavailable`.
     *
     * If you don't mind the function throws, use `get_proc_address()` instead.
     */
    Proc get_instance_proc_address_noexcept(VkInstance instance, const char* proc_name) noexcept;

    /**
     * @brief Check whether the specified queue family of the specified physical device supports presentation
     * to the platform GLFW was built for.
     *
     * @param instance The instance that the physical device belongs to.
     * @param device The physical device that the queue family belongs to.
     * @param queue_family The index of the queue family to query.
     * @return True if the queue family supports presentation, false otherwise.
     *
     * @thread_safety This function may be called from any thread.
     *
     * @throw error::NotInitialized If GLFW is not initialized yet.
     * @throw error::ApiUnavailable If Vulkan API is not available.
     * @throw error::PlatformError If a platform-specific error occurred.
     */
    bool get_physical_device_presentation_support(
        VkInstance       instance,
        VkPhysicalDevice device,
        uint32_t         queue_family
    );

    /**
     * @brief Create a vulkan surface for the specified window.
     *
     * @param window The window to create the surface for.
     * @param instance The Vulkan instance to create the surface in.
     * @param allocator The allocator to use or `nullptr` to use the default allocator.
     * @param surface Where to store the handle of the surface.
     * @return `VK_SUCCESS` if successful, or Vulkan error code if an error occurred.
     */
    VkResult create_surface(
        const Window&                window,
        VkInstance                   instance,
        const VkAllocationCallbacks* allocator,
        VkSurfaceKHR*                surface
    );
#endif    // VK_VERSION_1_0

#ifdef VULKAN_HPP
    /**
     * @brief Helper function for application that uses `vulkan.hpp` header.
     *
     * See the other overload of `get_instance_proc_address()` to see the documentation.
     */
    inline Proc get_instance_proc_address(const ::vk::Instance& instance, const char* proc_name)
    {
        return get_instance_proc_address(static_cast<VkInstance>(instance), proc_name);
    }

    /**
     * @brief Helper function for application that uses `vulkan.hpp` header.
     *
     * See the other overload of `get_instance_proc_address_noexcept()` to see the documentation.
     */
    inline Proc get_instance_proc_address_noexcept(
        const ::vk::Instance& instance,
        const char*           proc_name
    ) noexcept
    {
        return get_instance_proc_address_noexcept(static_cast<VkInstance>(instance), proc_name);
    }

    /**
     * @brief Helper function for application that uses `vulkan.hpp` header.
     *
     * See the other overload of `get_physical_device_presentation_support()` to see the documentation.
     */
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

    /**
     * @brief Helper function for application that uses `vulkan.hpp` header.
     *
     * See the other overload of `create_surface()` to see the documentation.
     */
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
