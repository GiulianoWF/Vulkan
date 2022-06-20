#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#include <vulkan/vulkan.h>

// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace bi = boost::interprocess;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

int main()
{
    // boost::interprocess::shared_memory_object mSharedMemoryObject;
    // boost::interprocess::mapped_region mMappedRegion;

    // mSharedMemoryObject = bi::shared_memory_object(bi::open_only
    //                                                     ,"SHAREDVERTEXBUFFER"
    //                                                     ,bi::read_write
    //                                                     );

    // mMappedRegion = bi::mapped_region(mSharedMemoryObject, bi::read_write);

    // float* iter = (float*)mMappedRegion.get_address();

    // while(true)
    // {
    //     for (int a=0; a < 100;)
    //     {
    //         std::cout << "{{"<< *(iter + a++) << "f, " <<*(iter + a++)<<"f, "<<*(iter + a++)<<"f}, {"<<*(iter + a++)<<"f, "<<*(iter + a++)<<"f, "<<*(iter + a++)<<"f}, {"<<*(iter + a++)<<"f, "<<*(iter + a++)<<"f}}" << std::endl;
    //     }
    //     std::cout << "\n\n\n\n\n\n\n";
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }

    // uint32_t memoryStagingIndex = gpu->getMemoryTypeIndex(my_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // VkMemoryHostPointerPropertiesEXT pMemoryHostPointerProperties { VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT, nullptr, 1 }; // side question: what should the '1' be really? The spec is confusingly worded

    // result = vkGetMemoryHostPointerPropertiesEXT( device,	VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, my_ptr, &pMemoryHostPointerProperties);
    VkInstance instance;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;



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
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

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

    auto isDeviceSuitable = [](VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate  && supportedFeatures.samplerAnisotropy;
    };


    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }



    VkPhysicalDeviceProperties pp;
    VkPhysicalDeviceProperties2 physprop;
    physprop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physprop.properties = pp;
    vkGetPhysicalDeviceProperties2(physicalDevice, &physprop);


}