#pragma once

class SwapChainProcesser
{
    public:
        VkSwapchainKHR              mSwapChain;
        std::vector<VkImage>        mSwapChainImages;
        VkFormat                    mSwapChainImageFormat;
        VkExtent2D                  mSwapChainExtent;
        std::vector<VkImageView>    mSwapChainImageViews;
        std::vector<VkFramebuffer>  mSwapChainFramebuffers;
};