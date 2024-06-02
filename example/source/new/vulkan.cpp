#include <fmt/core.h>
#include <glfw_cpp/glfw_cpp.hpp>
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>      // you need to include the vulkan header first
#include <glfw_cpp/vulkan.hpp>    // vulkan functionality for glfw_cpp is separated

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if !defined(NDEBUG)
#    define ENABLE_VULKAN_VALIDATION_LAYERS 1
#else
#    define ENABLE_VULKAN_VALIDATION_LAYERS 0
#endif

struct QueueFamilyIndices
{
public:
    uint32_t m_graphicsFamily;
    uint32_t m_presentFamily;

    static QueueFamilyIndices getCompleteQueueFamilies(
        const vk::PhysicalDevice& device,
        const vk::SurfaceKHR&     surface
    )
    {
        OptionalQueueFamilyIndices indices;

        std::vector queueFamilies{ device.getQueueFamilyProperties() };
        for (uint32_t index{ 0 }; const auto& queueFamiliy : queueFamilies) {
            if (queueFamiliy.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.m_graphicsFamily = index;
            }

            if (device.getSurfaceSupportKHR(index, surface) == VK_TRUE) {
                indices.m_presentFamily = index;
            }

            if (indices.isComplete()) {
                break;
            }
            ++index;
        }

        if (!indices.isComplete()) {
            throw std::runtime_error{
                "Queue family not complete for selected device (This should not happen!)"
            };
        }

        return QueueFamilyIndices{
            .m_graphicsFamily = indices.m_graphicsFamily.value(),
            .m_presentFamily  = indices.m_presentFamily.value(),
            /* ... */
        };
    }

    static bool checkCompleteness(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
    {
        OptionalQueueFamilyIndices indices;

        std::vector queueFamilies{ device.getQueueFamilyProperties() };
        for (uint32_t index{ 0 }; const auto& queueFamiliy : queueFamilies) {
            if (queueFamiliy.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.m_graphicsFamily = index;
            };

            if (device.getSurfaceSupportKHR(index, surface) == VK_TRUE) {
                indices.m_presentFamily = index;
            }

            if (indices.isComplete()) {
                return true;
            }
            ++index;
        }

        return indices.isComplete();
    }

    std::set<uint32_t> getUniqueIndices() const
    {
        return {
            m_graphicsFamily,
            m_presentFamily,
        };
    }

    auto asArray() const
    {
        return std::array{
            m_graphicsFamily,
            m_presentFamily,
        };
    }

private:
    struct OptionalQueueFamilyIndices
    {
        std::optional<uint32_t> m_graphicsFamily;
        std::optional<uint32_t> m_presentFamily;

        bool isComplete() const
        {
            return (m_graphicsFamily.has_value() && m_presentFamily.has_value());
        }
    };
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR        m_capabilities;
    std::vector<vk::SurfaceFormatKHR> m_formats;
    std::vector<vk::PresentModeKHR>   m_presentModes;

    bool isAdequate() const { return !m_formats.empty() && !m_presentModes.empty(); }

    vk::SurfaceFormatKHR chooseSurfaceFormat() const
    {
        // prefer srgb color space with 8-bit rgba
        for (const auto& format : m_formats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }

        // otherwise, just choose the first one
        return m_formats.front();
    }

    vk::PresentModeKHR choosePresentMode() const
    {
        for (const auto& mode : m_presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                return mode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseExtent(glfw_cpp::Window& window) const
    {
        // if the extent is not max uint32_t, then we can just use it
        if (m_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return m_capabilities.currentExtent;
        }

        // otherwise, set it manually
        auto [width, height] = window.properties().m_dimension;

        return {
            .width = std::clamp(
                static_cast<uint32_t>(width),
                m_capabilities.minImageExtent.width,
                m_capabilities.maxImageExtent.width
            ),
            .height = std::clamp(
                static_cast<uint32_t>(height),
                m_capabilities.minImageExtent.height,
                m_capabilities.maxImageExtent.height
            ),
        };
    }
};

class DebugMessenger
{
public:
    vk::Instance               m_instance{ nullptr };
    vk::DebugUtilsMessengerEXT m_debugMessenger;

    static DebugMessenger create(
        vk::Instance                                instance,
        const vk::DebugUtilsMessengerCreateInfoEXT& createInfo
    )
    {
        DebugMessenger            messenger;
        vk::DispatchLoaderDynamic dynamicDispatch{ instance, vkGetInstanceProcAddr };

        messenger.m_instance       = instance;
        messenger.m_debugMessenger = instance.createDebugUtilsMessengerEXT(
            createInfo, nullptr, dynamicDispatch
        );

        return messenger;
    }

    DebugMessenger()                                 = default;
    DebugMessenger(const DebugMessenger&)            = delete;
    DebugMessenger& operator=(const DebugMessenger&) = delete;

    DebugMessenger& operator=(DebugMessenger&& other) noexcept
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_debugMessenger, other.m_debugMessenger);
        return *this;
    }

    DebugMessenger(DebugMessenger&& other) noexcept
        : DebugMessenger()
    {
        std::swap(m_instance, other.m_instance);
        std::swap(m_debugMessenger, other.m_debugMessenger);
    }

    ~DebugMessenger()
    {
        if (!m_instance) {
            return;
        }

        auto dynamicDispatch = vk::DispatchLoaderDynamic{ m_instance, vkGetInstanceProcAddr };
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dynamicDispatch);
    }
};

struct SyncObject
{
    vk::UniqueSemaphore m_imageAvailableSemaphore;
    vk::UniqueSemaphore m_renderFinishedSemaphore;
    vk::UniqueFence     m_inFlightFence;
};

class Vulkan
{
public:
    Vulkan()                               = default;
    Vulkan(Vulkan&&)                       = default;
    Vulkan& operator=(Vulkan&&)            = default;
    Vulkan(const Vulkan&)                  = delete;
    Vulkan& operator=(const Vulkan& other) = delete;

    ~Vulkan()
    {
        if (!m_instance) {
            return;
        }

        // NOTE: ignoring possible throw--the only failure that may happen is when a catastrophic
        // failure occurs
        m_device->waitIdle();
    }

    Vulkan(glfw_cpp::Window& window, const std::string& name)
        : m_window{ &window }
    {
        std::vector extensions = glfw_cpp::vk::getRequiredInstanceExtensions();
        if constexpr (s_enableValidation) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // setup vulkan instance
        m_instance = createVulkanInstance(name, extensions);

        // setup debug messenger if validation layers are enabled
        if constexpr (s_enableValidation) {
            auto debugCreateInfo = getDebugMessengerCreateInfo();
            m_debugMessenger     = DebugMessenger::create(*m_instance, debugCreateInfo);
        }

        // setup window surface
        m_surface = createSurface(*m_window, *m_instance);

        // configure vulkan physical device
        m_physicalDevice = pickPhysicalDevice(*m_instance, *m_surface);

        fmt::println(
            "INFO: [Vulkan] Using physical device: {}",
            m_physicalDevice.getProperties().deviceName.begin()
        );

        // configure vulkan logical device and queues
        QueueFamilyIndices queueFamilies{
            QueueFamilyIndices::getCompleteQueueFamilies(m_physicalDevice, *m_surface)
        };
        m_device = createLogicalDevice(m_physicalDevice, queueFamilies);

        m_graphicsQueue = m_device->getQueue(queueFamilies.m_graphicsFamily, 0);
        m_presentQueue  = m_device->getQueue(queueFamilies.m_presentFamily, 0);

        // setup swap chain
        std::tie(m_swapChain, m_swapChainImageFormat, m_swapChainExtent) = createSwapChain(
            m_physicalDevice, *m_device, *m_surface, queueFamilies, *m_window
        );

        m_swapChainImages     = m_device->getSwapchainImagesKHR(*m_swapChain);
        m_swapChainImageViews = createImageViews(
            *m_device, m_swapChainImages, m_swapChainImageFormat
        );

        // create graphics pipeline
        m_renderPass = createRenderPass(*m_device, m_swapChainImageFormat);
        std::tie(m_pipelineLayout, m_graphicsPipeline) = createPipelineLayout(
            *m_device, *m_renderPass
        );

        // create framebuffers
        m_swapChainFramebuffers = createFramebuffers(
            *m_device, *m_renderPass, m_swapChainImageViews, m_swapChainExtent
        );

        m_commandPool    = createCommandPool(*m_device, queueFamilies);
        m_commandBuffers = createCommandBuffers(*m_device, *m_commandPool);

        // create sync objects
        m_syncs = createSyncObjects(*m_device);
    }

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
    {
        constexpr vk::ClearValue clearValue{ .color = { std::array{ 0.01F, 0.01F, 0.02F, 1.0F } } };

        vk::CommandBufferBeginInfo commandBeginInfo{
            .flags            = {},    //  use default flags
            .pInheritanceInfo = {},    //  only relevant for secondary command buffers
        };
        commandBuffer.begin(commandBeginInfo);

        vk::RenderPassBeginInfo renderPassBeginInfo{
            .renderPass  = *m_renderPass,
            .framebuffer = *m_swapChainFramebuffers[imageIndex],
            .renderArea  = {
                 .offset = { 0, 0 },
                 .extent = m_swapChainExtent,
            },
            .clearValueCount = 1,
            .pClearValues    = &clearValue,
        };
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);

        // NOTE: swap chain extent is in pixels, not in screen coordinates
        vk::Viewport viewport{
            .x        = 0.0F,
            .y        = 0.0F,
            .width    = static_cast<float>(m_swapChainExtent.width),
            .height   = static_cast<float>(m_swapChainExtent.height),
            .minDepth = 0.0F,
            .maxDepth = 1.0F,
        };
        commandBuffer.setViewport(0, viewport);

        vk::Rect2D scissor{
            .offset = { 0, 0 },
            .extent = m_swapChainExtent,
        };
        commandBuffer.setScissor(0, scissor);

        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();

        commandBuffer.end();
    }

    void drawFrame()
    {
        // wait forever, effectively disabling timeout
        constexpr auto timeoutNano{ std::numeric_limits<uint64_t>::max() };

        std::array fences{ *m_syncs[m_currentFrameIndex].m_inFlightFence };
        auto       fenceResult = m_device->waitForFences(fences, VK_TRUE, timeoutNano);
        if (fenceResult != vk::Result::eSuccess) {
            throw std::runtime_error(
                std::format("Failed to wait for fence: {}", vk::to_string(fenceResult))
            );
        }

        // acquire image from swap chain
        uint32_t imageIndex{};
        switch (auto [acquireResult, imageIndexTmp] = swapchainNextImageWrapper(
                    *m_device,
                    *m_swapChain,
                    timeoutNano,
                    *m_syncs[m_currentFrameIndex].m_imageAvailableSemaphore
                );
                acquireResult) {
        case vk::Result::eSuccess: {
            imageIndex = imageIndexTmp;
            break;
        }
        case vk::Result::eErrorOutOfDateKHR: [[fallthrough]];
        case vk::Result::eSuboptimalKHR: {
            recreateSwapChain();
            return;
        }
        default: {
            throw std::runtime_error(
                std::format("Failed to acquire swap chain image: {}", vk::to_string(acquireResult))
            );
        }
        };

        m_device->resetFences(fences);

        // record command buffer -- reset the command buffer not the unique pointer itself
        m_commandBuffers[m_currentFrameIndex]->reset();
        recordCommandBuffer(m_commandBuffers[m_currentFrameIndex].get(), imageIndex);

        // submit command buffer
        std::array<vk::Semaphore, 1> waitSemaphore{
            *m_syncs[m_currentFrameIndex].m_imageAvailableSemaphore
        };
        std::array<vk::PipelineStageFlags, 1> waitStages{
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };

        vk::SubmitInfo submitInfo{
            .waitSemaphoreCount   = waitSemaphore.size(),
            .pWaitSemaphores      = waitSemaphore.data(),
            .pWaitDstStageMask    = waitStages.data(),
            .commandBufferCount   = 1,
            .pCommandBuffers      = &m_commandBuffers[m_currentFrameIndex].get(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &m_syncs[m_currentFrameIndex].m_renderFinishedSemaphore.get(),
        };

        auto submitResult = m_graphicsQueue.submit(
            1, &submitInfo, *m_syncs[m_currentFrameIndex].m_inFlightFence
        );
        if (submitResult != vk::Result::eSuccess) {
            throw std::runtime_error(
                std::format("Failed to submit draw command buffer: {}", vk::to_string(submitResult))
            );
        }

        // present image to screen
        vk::PresentInfoKHR presentInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &m_syncs[m_currentFrameIndex].m_renderFinishedSemaphore.get(),
            .swapchainCount     = 1,
            .pSwapchains        = &m_swapChain.get(),
            .pImageIndices      = &imageIndex,
            .pResults           = nullptr,
        };

        switch (auto presentResult = queuePresentWrapper(m_presentQueue, presentInfo);
                presentResult) {
        case vk::Result::eSuccess: {
            break;
        }
        case vk::Result::eErrorOutOfDateKHR: [[fallthrough]];
        case vk::Result::eSuboptimalKHR: {
            recreateSwapChain();
            return;
        }
        default: {
            throw std::runtime_error(
                std::format("Failed to present swap chain image: {}", vk::to_string(presentResult))
            );
        }
        };

        m_currentFrameIndex = (m_currentFrameIndex + 1) % s_maxFramesInFlight;
    }

private:
    static constexpr bool        s_enableValidation  = ENABLE_VULKAN_VALIDATION_LAYERS;
    static constexpr std::array  s_validationLayers  = { "VK_LAYER_KHRONOS_validation" };
    static constexpr std::array  s_deviceExtensions  = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    static constexpr std::size_t s_maxFramesInFlight = 2;    // should not be zero, of course

    glfw_cpp::Window* m_window;

    vk::UniqueInstance   m_instance;
    vk::UniqueSurfaceKHR m_surface;
    DebugMessenger       m_debugMessenger;

    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueDevice   m_device;

    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;

    vk::UniqueSwapchainKHR             m_swapChain;
    std::vector<vk::Image>             m_swapChainImages;
    vk::Format                         m_swapChainImageFormat;
    vk::Extent2D                       m_swapChainExtent;
    std::vector<vk::UniqueImageView>   m_swapChainImageViews;
    std::vector<vk::UniqueFramebuffer> m_swapChainFramebuffers;

    vk::UniqueRenderPass     m_renderPass;
    vk::UniquePipelineLayout m_pipelineLayout;
    vk::UniquePipeline       m_graphicsPipeline;

    vk::UniqueCommandPool                m_commandPool;
    std::vector<vk::UniqueCommandBuffer> m_commandBuffers;
    std::vector<SyncObject>              m_syncs;

    uint32_t m_currentFrameIndex{ 0 };

    // NOTE: for some reason, I can't use the c++ bindings for this callback
    // (can't be assigned to vk::DebugUtilsMessengerCreateInfoEXT)
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
        VkDebugUtilsMessageTypeFlagsEXT             type [[maybe_unused]],
        const VkDebugUtilsMessengerCallbackDataEXT* pData,
        void*                                       pUserData [[maybe_unused]]
    )
    {
        using Severity               = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        constexpr auto to_underlying = [](Severity flag) {
            return static_cast<std::underlying_type_t<Severity>>(flag);
        };
        if (severity >= to_underlying(Severity::eVerbose)) {
            fmt::println("VKDEBUG: {}", pData->pMessage);
        }
        return VK_FALSE;
    }

    static vk::DebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo()
    {
        using s = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using t = vk::DebugUtilsMessageTypeFlagBitsEXT;
        return {
            .messageSeverity = s::eVerbose | s::eWarning | s::eError,    // | s::eInfo,
            .messageType     = t::eGeneral | t::eValidation | t::ePerformance,
            .pfnUserCallback = debugCallback,
            .pUserData       = nullptr,
        };
    }

    static vk::UniqueInstance createVulkanInstance(
        const std::string&       name,
        std::vector<const char*> extensions
    )
    {
        vk::ApplicationInfo appInfo{
            .pApplicationName   = name.c_str(),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "No Engine",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_0,
        };

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo        = &appInfo,
            .enabledLayerCount       = 0,    // disable validation layer for now
            .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;

        // enable debug utils extension
        if constexpr (s_enableValidation) {
            if (!checkValidationLayerSupport()) {
                throw std::runtime_error("Validation layers requested, but not available");
            }
            createInfo.enabledLayerCount   = s_validationLayers.size();
            createInfo.ppEnabledLayerNames = s_validationLayers.data();

            debugCreateInfo  = getDebugMessengerCreateInfo();
            createInfo.pNext = &debugCreateInfo;
        }

        return vk::createInstanceUnique(createInfo);
    }

    static bool checkValidationLayerSupport()
    {
        auto layers{ vk::enumerateInstanceLayerProperties() };

        for (const auto& layerName : s_validationLayers) {
            auto found = std::ranges::find_if(layers, [&layerName](const auto& layer) {
                return std::strcmp(layerName, layer.layerName) == 0;    // same string
            });
            if (found != layers.end()) {
                return true;
            }
        }
        return false;
    }

    static vk::PhysicalDevice pickPhysicalDevice(
        const vk::Instance&   instance,
        const vk::SurfaceKHR& surface
    )
    {
        auto physicalDevices{ instance.enumeratePhysicalDevices() };
        if (physicalDevices.empty()) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        // sorted from best to worst [O(nlogn))]
        std::multimap<int, vk::PhysicalDevice, std::greater<>> candidates;
        for (const auto& device : physicalDevices) {
            int score = rateDeviceSuitability(device, surface);
            if (score > 0) {
                candidates.emplace(score, device);
                fmt::println(
                    "INFO: [Vulkan] Found suitable GPU: {} (score: {})",
                    device.getProperties().deviceName.data(),
                    score
                );
            }
        }
        if (candidates.empty()) {
            throw std::runtime_error("Failed to find a suitable GPU");
        }
        return candidates.begin()->second;
    }

    static int rateDeviceSuitability(
        const vk::PhysicalDevice& device,
        const vk::SurfaceKHR&     surface
    )
    {
        int         score{ 0 };
        const auto& properties{ device.getProperties() };
        const auto& features{ device.getFeatures() };

        constexpr int SCORE_DISCRETE   = 1000;
        constexpr int SCORE_INTEGRATED = 100;
        constexpr int SCORE_VIRTUAL    = 10;
        constexpr int SCORE_CPU        = 1;

        // clang-format off
        // discrete gpu is better than integrated gpu
        switch (properties.deviceType) {
            using DT = vk::PhysicalDeviceType;
        case DT::eDiscreteGpu:   score += SCORE_DISCRETE;   break;
        case DT::eIntegratedGpu: score += SCORE_INTEGRATED; break;
        case DT::eVirtualGpu:    score += SCORE_VIRTUAL;    break;
        case DT::eCpu:           score += SCORE_CPU;        break;
        default: break;
        }
        // clang-format on

        // max image dimension affects texture graphics quality
        score += static_cast<int>(properties.limits.maxImageDimension2D);

        // geometry shader is required
        if (features.geometryShader == VK_FALSE) {
            return 0;
        }

        // check queue families completeness
        auto complete{ QueueFamilyIndices::checkCompleteness(device, surface) };
        if (!complete) {
            return 0;
        }

        // check device extension support
        if (!checkDeviceExtensionSupport(device)) {
            return 0;
        }

        // check swap chain support
        auto swapChainSupport{ querySwapChainSupport(device, surface) };
        if (!swapChainSupport.isAdequate()) {
            return 0;
        }

        return score;
    }

    static bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
    {
        auto availableExtensions{ device.enumerateDeviceExtensionProperties() };

        for (const auto& extension : s_deviceExtensions) {
            auto found = std::ranges::find_if(
                availableExtensions,
                [&extension](const auto& availableExtension) {
                    return std::strcmp(extension, availableExtension.extensionName) == 0;
                }
            );
            if (found == availableExtensions.end()) {
                return false;
            }
        }

        return true;
    }

    static vk::UniqueDevice createLogicalDevice(
        const vk::PhysicalDevice& physicalDevice,
        const QueueFamilyIndices& indices
    )
    {
        constexpr float queuePriority = 1.0F;

        auto uniqueIndices{ indices.getUniqueIndices() };

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueIndices.size());
        for (const auto& index : uniqueIndices) {
            queueCreateInfos.push_back({
                .queueFamilyIndex = index,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority,
            });
        }

        vk::PhysicalDeviceFeatures deviceFeatures{
            // use default values for now
        };

        vk::DeviceCreateInfo createInfo{
            .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos       = queueCreateInfos.data(),
            .enabledExtensionCount   = s_deviceExtensions.size(),
            .ppEnabledExtensionNames = s_deviceExtensions.data(),
            .pEnabledFeatures        = &deviceFeatures,
        };

        // setup validation layers (newer version of vulkan ignores this)
        if constexpr (s_enableValidation) {
            createInfo.enabledLayerCount   = s_validationLayers.size();
            createInfo.ppEnabledLayerNames = s_validationLayers.data();
        }

        return physicalDevice.createDeviceUnique(createInfo);
    }

    static vk::UniqueSurfaceKHR createSurface(
        glfw_cpp::Window&   window,
        const vk::Instance& instance
    )
    {
        auto [result, surface] = glfw_cpp::vk::createSurface(window, instance);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error(
                std::format("Failed to create window surface: {}", vk::to_string(result))
            );
        }
        return vk::UniqueSurfaceKHR{ surface, instance };
    }

    static SwapChainSupportDetails querySwapChainSupport(
        const vk::PhysicalDevice& device,
        const vk::SurfaceKHR&     surface
    )
    {
        return {
            .m_capabilities = device.getSurfaceCapabilitiesKHR(surface),
            .m_formats      = device.getSurfaceFormatsKHR(surface),
            .m_presentModes = device.getSurfacePresentModesKHR(surface),
        };
    }

    static std::tuple<vk::UniqueSwapchainKHR, vk::Format, vk::Extent2D> createSwapChain(
        const vk::PhysicalDevice& physicalDevice,
        const vk::Device&         device,
        const vk::SurfaceKHR&     surface,
        const QueueFamilyIndices& queueIndices,
        glfw_cpp::Window&         window
    )
    {
        const auto swapChainSupport{ querySwapChainSupport(physicalDevice, surface) };
        const auto surfaceFormat{ swapChainSupport.chooseSurfaceFormat() };
        const auto presentMode{ swapChainSupport.choosePresentMode() };
        const auto extent{ swapChainSupport.chooseExtent(window) };

        uint32_t imageCount{ swapChainSupport.m_capabilities.minImageCount + 1 };
        if (swapChainSupport.m_capabilities.maxImageCount > 0
            && imageCount > swapChainSupport.m_capabilities.maxImageCount) {
            imageCount = swapChainSupport.m_capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{
            .surface          = surface,
            .minImageCount    = imageCount,
            .imageFormat      = surfaceFormat.format,
            .imageColorSpace  = surfaceFormat.colorSpace,
            .imageExtent      = extent,
            .imageArrayLayers = 1,
            .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
            .preTransform     = swapChainSupport.m_capabilities.currentTransform,
            .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode      = presentMode,
            .clipped          = VK_TRUE,
            .oldSwapchain     = nullptr,
        };

        if (queueIndices.m_graphicsFamily != queueIndices.m_presentFamily) {
            createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueIndices.asArray().data();
        } else {
            createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices   = nullptr;
        }

        auto swapChain{ device.createSwapchainKHRUnique(createInfo) };
        return { std::move(swapChain), surfaceFormat.format, extent };
    }

    static std::vector<vk::UniqueImageView> createImageViews(
        const vk::Device&             device,
        const std::vector<vk::Image>& swapchainImages,
        const vk::Format&             swapchainFormat
    )
    {
        std::vector<vk::UniqueImageView> imageViews;
        imageViews.reserve(swapchainImages.size());

        for (const auto& image : swapchainImages) {
            vk::ImageViewCreateInfo createInfo{
                .image      = image,
                .viewType   = vk::ImageViewType::e2D,
                .format     = swapchainFormat,
                .components = {
                    .r = vk::ComponentSwizzle::eIdentity,
                    .g = vk::ComponentSwizzle::eIdentity,
                    .b = vk::ComponentSwizzle::eIdentity,
                    .a = vk::ComponentSwizzle::eIdentity,
                },
                .subresourceRange = {
                    .aspectMask     = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1,
                },
            };
            imageViews.push_back(device.createImageViewUnique(createInfo));
        }

        return imageViews;
    }

    static vk::UniqueRenderPass createRenderPass(
        const vk::Device& device,
        const vk::Format& swapChainImageFormat
    )
    {
        vk::AttachmentDescription colorAttachment{
            .format         = swapChainImageFormat,
            .samples        = vk::SampleCountFlagBits::e1,
            .loadOp         = vk::AttachmentLoadOp::eClear,
            .storeOp        = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout  = vk::ImageLayout::eUndefined,
            .finalLayout    = vk::ImageLayout::ePresentSrcKHR,
        };

        vk::AttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout     = vk::ImageLayout::eColorAttachmentOptimal,
        };

        vk::SubpassDescription subpassDesciption{
            .pipelineBindPoint    = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &colorAttachmentRef,
        };

        vk::SubpassDependency subpassDependency{
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        };

        vk::RenderPassCreateInfo renderPassInfo{
            .attachmentCount = 1,
            .pAttachments    = &colorAttachment,
            .subpassCount    = 1,
            .pSubpasses      = &subpassDesciption,
            .dependencyCount = 1,
            .pDependencies   = &subpassDependency,
        };

        return device.createRenderPassUnique(renderPassInfo);
    }

    static std::pair<vk::UniquePipelineLayout, vk::UniquePipeline> createPipelineLayout(
        vk::Device&     device,
        vk::RenderPass& renderPass
    )
    {
        const auto loadShader = [](std::filesystem::path&& filepath) {
            if (!std::filesystem::exists(filepath)) {
                throw std::runtime_error(std::format("No such file '{}'", filepath.string()));
            }

            std::ifstream file{ filepath.string(), std::ios::ate | std::ios::binary };
            if (!file) {
                throw std::runtime_error("Failed to open file " + filepath.string());
            }

            auto              fileSize{ static_cast<std::size_t>(file.tellg()) };
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

            return buffer;
        };

        const auto createShaderModule = [&device](const std::vector<char>& shaderCode) {
            vk::ShaderModuleCreateInfo createInfo{
                .codeSize = shaderCode.size(),
                .pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data()),    // hack
            };
            return device.createShaderModuleUnique(createInfo);
        };

        auto vertShaderCode{ loadShader("asset/shader/vert.spv") };
        auto fragShaderCode{ loadShader("asset/shader/frag.spv") };

        vk::UniqueShaderModule vertShaderModule{ createShaderModule(vertShaderCode) };
        vk::UniqueShaderModule fragShaderModule{ createShaderModule(fragShaderCode) };

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage               = vk::ShaderStageFlagBits::eVertex,
            .module              = vertShaderModule.get(),
            .pName               = "main",    // entry point
            .pSpecializationInfo = nullptr,
        };
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage               = vk::ShaderStageFlagBits::eFragment,
            .module              = fragShaderModule.get(),
            .pName               = "main",
            .pSpecializationInfo = nullptr,
        };
        std::array shaderStages{
            vertShaderStageInfo,
            fragShaderStageInfo,
        };

        // describe vertex input (none at the moment)
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount   = 0,
            .pVertexBindingDescriptions      = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions    = nullptr,
        };

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
            .topology               = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = VK_FALSE,
        };

        // dynamic viewport and scissor states
        std::array dynamicStates{
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates    = dynamicStates.data(),
        };
        vk::PipelineViewportStateCreateInfo viewportStateInfo{
            .viewportCount = 1,
            .scissorCount  = 1,
        };

        // rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizerInfo{
            .depthClampEnable        = VK_FALSE,    // enabling this requires enabling gpu feature
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = vk::PolygonMode::eFill,
            .cullMode                = vk::CullModeFlagBits::eBack,
            .frontFace               = vk::FrontFace::eClockwise,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = 1.0F,
        };

        // multisampling (disable for now)
        vk::PipelineMultisampleStateCreateInfo multisamplingInfo{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable  = VK_FALSE,
        };

        // depth and stencil testing (none for now)
        /* literally nothing */

        // color blending
        using c = vk::ColorComponentFlagBits;
        vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable    = VK_FALSE,
            .colorWriteMask = c::eR | c::eG | c::eB | c::eA,
        };

        vk::PipelineColorBlendStateCreateInfo colorBlendInfo{
            .logicOpEnable   = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments    = &colorBlendAttachment,
            .blendConstants  = std::array{ 0.0F, 0.0F, 0.0F, 0.0F },
        };

        // pipeline layout (empty for now)
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount         = 0,
            .pSetLayouts            = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };

        auto pipelineLayout{ device.createPipelineLayoutUnique(pipelineLayoutInfo) };

        // finally, the real pipeline creation
        vk::GraphicsPipelineCreateInfo graphicsPipelineInfo{
            .stageCount          = static_cast<uint32_t>(shaderStages.size()),
            .pStages             = shaderStages.data(),
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pViewportState      = &viewportStateInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState   = &multisamplingInfo,
            .pDepthStencilState  = nullptr,    // no depth and stencil testing for now
            .pColorBlendState    = &colorBlendInfo,
            .pDynamicState       = &dynamicStateInfo,
            .layout              = *pipelineLayout,
            .renderPass          = renderPass,
            .subpass             = 0,
        };

        // NOTE: it seems that this function returns a result while also throwing on failure
        auto [result, graphicsPipeline] = device.createGraphicsPipelineUnique(
            nullptr, graphicsPipelineInfo
        );
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error(
                std::format("Failed to create graphics pipeline: {}", vk::to_string(result))
            );
        }
        return { std::move(pipelineLayout), std::move(graphicsPipeline) };
    }

    static std::vector<vk::UniqueFramebuffer> createFramebuffers(
        const vk::Device&                       device,
        const vk::RenderPass&                   renderPass,
        const std::vector<vk::UniqueImageView>& swapChainImageViews,
        const vk::Extent2D&                     swapChainExtent
    )
    {
        std::vector<vk::UniqueFramebuffer> framebuffers;

        framebuffers.reserve(swapChainImageViews.size());
        for (const auto& imageView : swapChainImageViews) {
            vk::FramebufferCreateInfo framebufferInfo{
                .renderPass      = renderPass,
                .attachmentCount = 1,
                .pAttachments    = &imageView.get(),
                .width           = swapChainExtent.width,
                .height          = swapChainExtent.height,
                .layers          = 1,
            };
            framebuffers.push_back(device.createFramebufferUnique(framebufferInfo));
        }

        return framebuffers;
    }

    static vk::UniqueCommandPool createCommandPool(
        const vk::Device&         device,
        const QueueFamilyIndices& queueFamilyIndices
    )
    {
        vk::CommandPoolCreateInfo commandPoolInfo{
            .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamilyIndices.m_graphicsFamily,
        };

        return device.createCommandPoolUnique(commandPoolInfo);
    }

    static std::vector<vk::UniqueCommandBuffer> createCommandBuffers(
        const vk::Device&      device,
        const vk::CommandPool& commandPool
    )
    {
        vk::CommandBufferAllocateInfo commandBufferInfo{
            .commandPool        = commandPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = s_maxFramesInFlight,
        };

        return device.allocateCommandBuffersUnique(commandBufferInfo);
    }

    static std::vector<SyncObject> createSyncObjects(const vk::Device& device)
    {
        vk::SemaphoreCreateInfo semaphoreInfo{
            .flags = {},    // default for now
        };
        vk::FenceCreateInfo fenceInfo{
            .flags = vk::FenceCreateFlagBits::eSignaled,    // start with signaled state
        };

        std::vector<SyncObject> syncObjects(s_maxFramesInFlight);
        for (auto& sync : syncObjects) {
            sync = {
                .m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreInfo),
                .m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo),
                .m_inFlightFence           = device.createFenceUnique(fenceInfo),
            };
        }

        return syncObjects;
    }

    /**
     * @brief vk::SwapchainKHR::acquireNextImageKHR without exceptions
     */
    static vk::ResultValue<uint32_t> swapchainNextImageWrapper(
        const vk::Device&       device,
        const vk::SwapchainKHR& swapchain,
        uint64_t                timeoutNano,
        vk::Semaphore           semaphore
    )
    {
        uint32_t imageIndex;

        auto result = vkAcquireNextImageKHR(
            static_cast<VkDevice>(device),
            static_cast<VkSwapchainKHR>(swapchain),
            timeoutNano,
            semaphore,
            nullptr,
            &imageIndex
        );

        return { static_cast<vk::Result>(result), imageIndex };
    }

    /**
     * @brief vk::Queue::presentKHR without exceptions
     */
    static vk::Result queuePresentWrapper(
        const vk::Queue&          queue,
        const vk::PresentInfoKHR& presentInfo
    )
    {
        auto result = vkQueuePresentKHR(
            static_cast<VkQueue>(queue), reinterpret_cast<const VkPresentInfoKHR*>(&presentInfo)
        );
        return static_cast<vk::Result>(result);
    }

    void recreateSwapChain()
    {
        m_device->waitIdle();

        m_swapChainFramebuffers.clear();
        m_swapChainImageViews.clear();
        m_swapChain.reset();

        // recreate swap chain
        auto queueIndices{
            QueueFamilyIndices::getCompleteQueueFamilies(m_physicalDevice, *m_surface)
        };
        std::tie(m_swapChain, m_swapChainImageFormat, m_swapChainExtent) = createSwapChain(
            m_physicalDevice, *m_device, *m_surface, queueIndices, *m_window
        );
        m_swapChainImages     = m_device->getSwapchainImagesKHR(*m_swapChain);
        m_swapChainImageViews = createImageViews(
            *m_device, m_swapChainImages, m_swapChainImageFormat
        );
        m_swapChainFramebuffers = createFramebuffers(
            *m_device, *m_renderPass, m_swapChainImageViews, m_swapChainExtent
        );
    }
};

int main()
{
    // tip: search for glfw_cpp word inside this file to see how it is used

    auto glfwLogger = [](glfw_cpp::Instance::LogLevel level, std::string&& message) {
        fmt::println("LOG: [glfw-cpp: {}] {}", (int)level, std::move(message));
    };

    auto glfw   = glfw_cpp::init(glfw_cpp::Api::NoApi{}, glfwLogger);
    auto wm     = glfw->createWindowManager();
    auto window = wm.createWindow({}, "Learn vulkan", 800, 600);

    Vulkan vulkan{ window, "vulkan program" };

    window.run([&](const auto& events) {
        for (const glfw_cpp::Event& event : events) {
            if (auto* e = event.getIf<glfw_cpp::Event::KeyPressed>()) {
                if (e->m_key == glfw_cpp::KeyCode::Q) {
                    window.requestClose();
                }
            }
        }

        vulkan.drawFrame();
        wm.pollEvents();
    });
}
