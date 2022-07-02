VkDeviceSize GetMinImportedHostPointerAlignment(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT externalMemHostProp;
    externalMemHostProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;
    externalMemHostProp.pNext = nullptr;

    VkPhysicalDeviceProperties physicalDeviceProps;

    VkPhysicalDeviceProperties2 physicalDeviceProps2;
    physicalDeviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProps2.pNext = &externalMemHostProp;
    physicalDeviceProps2.properties = physicalDeviceProps;

    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProps2);

    return externalMemHostProp.minImportedHostPointerAlignment;
}

void printError(VkResult result)
{
    if (result != VK_SUCCESS) 
    {
        switch (result)
        {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            std::cout << "VK_ERROR_INVALID_EXTERNAL_HANDLE" << std::endl;
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR:
            std::cout << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR" << std::endl;
            break;
        default:
            break;
        }
    }
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void createAllocatedBuffer(VkPhysicalDevice physicalDevice, 
                           VkDevice const& device,
                           void* data,
                           VkBuffer & buffer,
                           VkDeviceMemory& bufferMemory,
                           VkDeviceSize size)
{
    // uint32_t extNum;
    // vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extNum, nullptr);
    // VkExtensionProperties deviceProps[extNum];
    // vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extNum, deviceProps);

    // std::cout << "Extensions " << extNum << std::endl;
    // for (int i =0 ; i < extNum ; ++i)
    // {
    //     // std::cout << deviceProps[i].extensionName << std::endl;
    // }

    // uint32_t extNum;
    // vkEnumerateInstanceExtensionProperties(nullptr, &extNum, nullptr);
    // VkExtensionProperties instanceProps[extNum];
    // vkEnumerateInstanceExtensionProperties(nullptr, &extNum, instanceProps);

    // std::cout << "Extensions " << extNum << std::endl;
    // for (int i =0 ; i < extNum ; ++i)
    // {
    //     std::cout << instanceProps[i].extensionName << std::endl;
    // }

    VkExternalMemoryHandleTypeFlagBits externalMemoryFlagBits {
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT
    };

    VkMemoryHostPointerPropertiesEXT pointersProps {
        .sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT,
        .pNext = nullptr,
    };

    vkGetMemoryHostPointerPropertiesEXT(device, 
                                        externalMemoryFlagBits,
                                        data,
                                        &pointersProps);

    VkExternalMemoryBufferCreateInfo externalMemCreateInfo {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .handleTypes = externalMemoryFlagBits,
    };

    VkBufferUsageFlags bufferUsage {
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = &externalMemCreateInfo,
        .flags = 0,
        .size = size,
        .usage = bufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryHostPointerPropertiesEXT memoryHostPointerProperties {
        .sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT,
    };

    if (auto result = vkGetMemoryHostPointerPropertiesEXT(device, externalMemoryFlagBits, data, &memoryHostPointerProperties); 
        result != VK_SUCCESS) 
    {
        printError(result);
    }
    std::cout << "VkMemoryHostPointerPropertiesEXT::memoryTypeBits " << memoryHostPointerProperties.memoryTypeBits << std::endl;

    VkMemoryPropertyFlags memProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VkImportMemoryHostPointerInfoEXT importHostPointerInfo {
        .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
        .pNext = nullptr,
        .handleType = externalMemoryFlagBits,
        .pHostPointer = data,
    };

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    auto memTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, memProperties);
    
    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &importHostPointerInfo,
        .allocationSize = size,
        .memoryTypeIndex = memTypeIndex,
    };

    VkDeviceMemory * pMemory;
    if (auto result = vkAllocateMemory(device, &allocInfo, nullptr, pMemory); 
        result != VK_SUCCESS) 
    {
        printError(result);
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

    // VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProps;
    // vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProps);
    // for(auto& memoryType : physicalDeviceMemoryProps.memoryTypes)
    // {
    //     // Is == 15;
    //     if(memoryType.propertyFlags == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
    //     {
    //         std::cout << "found" << std::endl;
    //     }
    // }
    // std::cout << "Type " << (int16_t)physicalDeviceMemoryProps.memoryTypes[0].propertyFlags << std::endl;

// VkBufferCreateFlags createBufferFlags;
// VkBufferUsageFlagBits bufferUsage;

// // Get VkBufferCreateInfo compatible handle types. VUID-VkBufferCreateInfo-pNext-00920
// VkPhysicalDeviceExternalBufferInfo physicalDeviceExternalBufferInfo;
// physicalDeviceExternalBufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
// physicalDeviceExternalBufferInfo.pNext = nullptr;
// physicalDeviceExternalBufferInfo.flags = createBufferFlags;
// physicalDeviceExternalBufferInfo.usage = bufferUsage;
// physicalDeviceExternalBufferInfo.handleType = externalMemoryFlagBits;

// VkExternalBufferProperties externalBufferProperties; // Getting VkExternalBufferProperties::externalMemoryProperties.compatibleHandleTypes to put on VkExternalMemoryBufferCreateInfo::handleTypes
// vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, &physicalDeviceExternalBufferInfo, &externalBufferProperties);

// void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//     VkBufferCreateInfo bufferInfo{};
//     bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//     bufferInfo.size = size;
//     bufferInfo.usage = usage;
//     bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//     if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create buffer!");
//     }

//     VkMemoryRequirements memRequirements;
//     vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

//     VkMemoryAllocateInfo allocInfo{};
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memRequirements.size;
//     allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

//     if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//         throw std::runtime_error("failed to allocate buffer memory!");
//     }

//     vkBindBufferMemory(device, buffer, bufferMemory, 0);
// }