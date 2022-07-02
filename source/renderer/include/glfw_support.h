#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class GlfwSupport
{
    protected:

    GLFWwindow* pWindow;

    GLFWframebuffersizefun pFramebufferResizeCallback = nullptr;

    void mInitWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(pWindow, this);
        glfwSetFramebufferSizeCallback(pWindow, pFramebufferResizeCallback);
    }

    void mCreateSurface(VkInstance& instance, VkSurfaceKHR& surface) {
        if (glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void mGetWindowRequiredExtensions(std::vector<const char*> & extensions)
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> glfwextensionsVec(glfwExtensions, glfwExtensions + glfwExtensionCount);

        for(const auto& extension : glfwextensionsVec)
        {
            extensions.push_back(extension);
        }
    }

    void mGetFramebufferSize(int & width, int & height)
    {
        return glfwGetFramebufferSize(pWindow, &width, &height);
    }

    bool mWindowShouldClose()
    {
        return glfwWindowShouldClose(pWindow);
    }

    void mPoolWindowEvents()
    {
        glfwPollEvents();
    }

    void mUpdateFramebufferSize()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(pWindow, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(pWindow, &width, &height);
            glfwWaitEvents();
        }
    }

    void mDestroyWindow()
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }
};
