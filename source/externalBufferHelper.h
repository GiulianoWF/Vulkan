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

void createAllocatedBuffer(VkPhysicalDevice physicalDevice, VkDevice const& device, void* data, VkBuffer & buffer, VkDeviceMemory& bufferMemory, VkDeviceSize size)
{
    std::cout << data << "   size " << size << std::endl;

    VkExternalMemoryHandleTypeFlagBits externalMemoryFlagBits
    {
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT
    };
    // pointerFlagBits = (VkExternalMemoryHandleTypeFlagBits) (pointerFlagBits | );
    // pointerFlagBits = (VkExternalMemoryHandleTypeFlagBits) (pointerFlagBits | VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT);

    VkMemoryHostPointerPropertiesEXT pointersProps;
    pointersProps.sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;
    pointersProps.pNext = nullptr;

    vkGetMemoryHostPointerPropertiesEXT(device, 
                                        externalMemoryFlagBits,
                                        data,
                                        &pointersProps); //Get pointer info.

    std::cout << "Got external host pointer info. Creating the buffer." << std::endl;

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

    VkExternalMemoryBufferCreateInfo externalMemCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .handleTypes = externalMemoryFlagBits,
    };

    VkBufferUsageFlags bufferUsage{VK_BUFFER_USAGE_TRANSFER_SRC_BIT};

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = &externalMemCreateInfo,
        .flags = 0,
        .size = size,
        .usage = bufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProps);
    for(auto& memoryType : physicalDeviceMemoryProps.memoryTypes)
    {
        // Is == 15;
        if(memoryType.propertyFlags == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
        {
            std::cout << "found" << std::endl;
        }
    }
    std::cout << "Type " << (int16_t)physicalDeviceMemoryProps.memoryTypes[0].propertyFlags << std::endl;

    VkImportMemoryHostPointerInfoEXT importHostPointerInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
        .pNext = nullptr,
        .handleType = externalMemoryFlagBits,
        .pHostPointer = data,
    };

    VkMemoryAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &importHostPointerInfo,
        .allocationSize = size,
        .memoryTypeIndex = 0,
    };

    VkMemoryHostPointerPropertiesEXT memoryHostPointerProperties
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT,
    };
    if (auto error = vkGetMemoryHostPointerPropertiesEXT(device, externalMemoryFlagBits, data, &memoryHostPointerProperties); error != VK_SUCCESS) 
    {
        switch (error)
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
    std::cout << "VkMemoryHostPointerPropertiesEXT::memoryTypeBits " << memoryHostPointerProperties.memoryTypeBits << std::endl;

    VkDeviceMemory pMemory;
    if (auto error = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory); error != VK_SUCCESS) {
        switch (error)
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
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

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