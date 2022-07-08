#pragma once

class SwapChainProcesser
{
    public:
        VkSwapchainKHR               m_cSwapChain;
        std::vector<VkImage>         m_cSwapChainImages;
        VkFormat                     m_cSwapChainImageFormat;
        VkExtent2D                   m_cSwapChainExtent;
        std::vector<VkImageView>     m_cSwapChainImageViews;
        std::vector<VkFramebuffer>   m_cSwapChainFramebuffers;

        VkDevice * m_cDevice;
        VkPhysicalDevice * m_cPhysicalDevice;
        VkSurfaceKHR * m_cSurface;

        VkImage m_cDepthImage;
        VkDeviceMemory m_cDepthImageMemory;
        VkImageView m_cDepthImageView;

        VkPipeline * m_cGraphicsPipeline;
        VkPipelineLayout * m_cPipelineLayout;
        VkRenderPass * m_cRenderPass;
    
    SwapChainProcesser( VkDevice * device,
                        VkPhysicalDevice * physicalDevice, 
                        VkSurfaceKHR * surface,
                        VkRenderPass * renderPass,
                        VkPipelineLayout * pipelineLayout,
                        VkPipeline * graphicsPipeline)
        : m_cDevice(device)
        , m_cPhysicalDevice(physicalDevice)
        , m_cSurface(surface)
        , m_cRenderPass(renderPass)
        , m_cPipelineLayout(pipelineLayout)
        , m_cGraphicsPipeline(graphicsPipeline)
    {

    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*m_cPhysicalDevice);

        VkSurfaceFormatKHR m_cSurfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = *m_cSurface,
            .minImageCount = imageCount,
            .imageFormat = m_cSurfaceFormat.format,
            .imageColorSpace = m_cSurfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };

        QueueFamilyIndices indices = findQueueFamilies(*m_cPhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(*m_cDevice, &createInfo, nullptr, &m_cSwapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(*m_cDevice, m_cSwapChain, &imageCount, nullptr);
        m_cSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(*m_cDevice, m_cSwapChain, &imageCount, m_cSwapChainImages.data());

        m_cSwapChainImageFormat = m_cSurfaceFormat.format;
        m_cSwapChainExtent = extent;
    }

    void cleanupSwapChain() {
        vkDestroyImageView(*m_cDevice, m_cDepthImageView, nullptr);
        vkDestroyImage(*m_cDevice, m_cDepthImage, nullptr);
        vkFreeMemory(*m_cDevice, m_cDepthImageMemory, nullptr);

        for (auto framebuffer : m_cSwapChainFramebuffers) {
            vkDestroyFramebuffer(*m_cDevice, framebuffer, nullptr);
        }

        vkDestroyPipeline(*m_cDevice, *m_cGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(*m_cDevice, *m_cPipelineLayout, nullptr);
        vkDestroyRenderPass(*m_cDevice, *m_cRenderPass, nullptr);

        for (auto imageView : m_cSwapChainImageViews) {
            vkDestroyImageView(*m_cDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(*m_cDevice, m_cSwapChain, nullptr);
    }

    void CreateImageViews()
    {
        m_cSwapChainImageViews.resize(m_cSwapChainImages.size());

        for (uint32_t i = 0; i < m_cSwapChainImages.size(); i++) {
            m_cSwapChainImageViews[i] = createImageView(m_cSwapChainImages[i], m_cSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(*m_cDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void AddImageView(size_t position, VkImageView imageView)
    {
        if(position >= m_cSwapChainImages.size())
        {
            throw std::overflow_error("Swap image index out of range");
        }

        m_cSwapChainImageViews[position] = imageView;
    }

    void createFramebuffers() {
        m_cSwapChainFramebuffers.resize(m_cSwapChainImageViews.size());

        for (size_t i = 0; i < m_cSwapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                m_cSwapChainImageViews[i],
                m_cDepthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *m_cRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_cSwapChainExtent.width;
            framebufferInfo.height = m_cSwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(*m_cDevice, &framebufferInfo, nullptr, &m_cSwapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *m_cSurface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *m_cSurface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_cSurface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_cSurface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_cSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_cSurface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else 
        {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(m_cSwapChainExtent.height),
                static_cast<uint32_t>(m_cSwapChainExtent.width)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    auto HasAdequateSwap(VkPhysicalDevice device) -> bool
    {
        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

        return swapChainAdequate;
    }
};
