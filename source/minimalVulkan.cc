#include <iostream>
#include <vector>
#include <optional>
#include <stdexcept>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

VkInstance instance;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;

int main()
{
    std::vector<const char*> extensions;
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        // extensions.push_back("VK_KHR_external_memory");
        extensions.push_back("VK_KHR_device_group_creation");
        extensions.push_back("VK_KHR_display");
        extensions.push_back("VK_KHR_external_fence_capabilities");
        extensions.push_back("VK_KHR_external_memory_capabilities");
        extensions.push_back("VK_KHR_external_semaphore_capabilities");
        extensions.push_back("VK_KHR_get_display_properties2");
        extensions.push_back("VK_KHR_get_physical_device_properties2");
        extensions.push_back("VK_KHR_get_surface_capabilities2");
        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_surface_protected_capabilities");
        extensions.push_back("VK_KHR_wayland_surface");
        extensions.push_back("VK_KHR_xcb_surface");
        extensions.push_back("VK_KHR_xlib_surface");
        extensions.push_back("VK_EXT_acquire_drm_display");
        extensions.push_back("VK_EXT_acquire_xlib_display");
        extensions.push_back("VK_EXT_debug_report");
        extensions.push_back("VK_EXT_direct_mode_display");
        extensions.push_back("VK_EXT_display_surface_counter");
        // extensions.push_back("VK_EXT_debug_utils");
        extensions.push_back("VK_KHR_portability_enumeration");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_VERSION_1_1;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = 0;

    createInfo.pNext = nullptr;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(auto& device : devices)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device , &props);
        std::cout << props.deviceName << std::endl;
    }
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[0] , &props);
        std::cout << "Selected: " << props.deviceName << std::endl;
    }

    physicalDevice = devices[0];

    VkPhysicalDeviceProperties physDeviceProp;
    VkPhysicalDeviceProperties2 physDeviceProp2;
    physDeviceProp2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physDeviceProp2.properties = physDeviceProp;
    vkGetPhysicalDeviceProperties2(physicalDevice, &physDeviceProp2);
}