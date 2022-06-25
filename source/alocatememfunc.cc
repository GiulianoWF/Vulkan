VKTRACER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL __HOOKED_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                                         const VkAllocationCallbacks* pAllocator,
                                                                         VkDeviceMemory* pMemory) {
    trim::TraceLock<std::mutex> lock(g_mutex_trace);
    VkResult result;
    size_t additional_size = 0;
    vktrace_trace_packet_header* pHeader;
    packet_vkAllocateMemory* pPacket = NULL;

    // If user disable using shadow memory, we'll use host memory through
    // VK_EXT_external_memory_host extension. If user enable using shadow
    // memory, there's no need to handle vkAllocateMemory because we don't
    // need to decide using the extension or not through memory property
    // flag.
    if (UseMappedExternalHostMemoryExtension()) {
        pageguardEnter();
    }

    size_t packetSize = get_struct_chain_size((void*)pAllocateInfo) + ROUNDUP_TO_4(sizeof(VkMemoryOpaqueCaptureAddressAllocateInfo)) + sizeof(VkAllocationCallbacks) + sizeof(VkDeviceMemory) * 2;
    CREATE_TRACE_PACKET(vkAllocateMemory, (packetSize + additional_size));

    VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo = {};
    void* pNextOriginal = const_cast<void*>(pAllocateInfo->pNext);
    void* pHostPointer = nullptr;
    VkDeviceSize original_allocation_size = pAllocateInfo->allocationSize;
    if (UseMappedExternalHostMemoryExtension()) {
        VkMemoryPropertyFlags propertyFlags = getPageGuardControlInstance().getMemoryPropertyFlags(device, pAllocateInfo->memoryTypeIndex);
        if ((propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
            // host visible memory, enable extension
            if (pAllocateInfo->pNext != nullptr) {
                // Todo: add checking process to detect if the extension we
                //      use here is compatible with the existing pNext chain,
                //      and output warning message if not compatible.
                assert(false);
            }

            // we insert our extension struct into the pNext chain.
            void** ppNext = const_cast<void**>(&(pAllocateInfo->pNext));
            *ppNext = &importMemoryHostPointerInfo;
            reinterpret_cast<VkImportMemoryHostPointerInfoEXT*>(*ppNext)->sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
            reinterpret_cast<VkImportMemoryHostPointerInfoEXT*>(*ppNext)->handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
            reinterpret_cast<VkImportMemoryHostPointerInfoEXT*>(*ppNext)->pNext = pNextOriginal;
            // provide the title host memory pointer which is already added
            // memory write watch.
            size_t page_size = pageguardGetSystemPageSize();
            if ((original_allocation_size % page_size) != 0) {
                const_cast<VkMemoryAllocateInfo*>(pAllocateInfo)->allocationSize = pAllocateInfo->allocationSize - (pAllocateInfo->allocationSize % page_size) + page_size;
            }
            reinterpret_cast<VkImportMemoryHostPointerInfoEXT*>(*ppNext)->pHostPointer = pageguardAllocateMemory(pAllocateInfo->allocationSize);
            pHostPointer = reinterpret_cast<VkImportMemoryHostPointerInfoEXT*>(*ppNext)->pHostPointer;
        }
    }

    VkMemoryAllocateFlagsInfo *allocateFlagInfo = (VkMemoryAllocateFlagsInfo*)find_ext_struct((const vulkan_struct_header*)pAllocateInfo->pNext, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO);
    if (allocateFlagInfo == nullptr) {
        allocateFlagInfo = (VkMemoryAllocateFlagsInfo*)find_ext_struct((const vulkan_struct_header*)pAllocateInfo->pNext, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR);
    }
    if (allocateFlagInfo != nullptr && (allocateFlagInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR)) {
        auto it = g_deviceToFeatureSupport.find(device);
        if (it != g_deviceToFeatureSupport.end()) {
            if (it->second.bufferDeviceAddressCaptureReplay) {
                allocateFlagInfo->flags = allocateFlagInfo->flags | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
            } else {
                vktrace_LogDebug("The device doesn't support bufferDeviceAddressCaptureReplay feature.");
            }
        }
    }

    result = mdd(device)->devTable.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    const_cast<VkMemoryAllocateInfo*>(pAllocateInfo)->allocationSize = original_allocation_size;
    if (UseMappedExternalHostMemoryExtension()) {
        if (result == VK_SUCCESS) {
            getPageGuardControlInstance().vkAllocateMemoryPageGuardHandle(device, pAllocateInfo, pAllocator, pMemory, pHostPointer);
            void** ppNext = const_cast<void**>(&(pAllocateInfo->pNext));
            // after allocation, we restore the original pNext. the extension
            // we insert here should not appear during playback.
            *ppNext = pNextOriginal;
        } else {
            if (pHostPointer) {
                pageguardFreeMemory(pHostPointer);
                pHostPointer = nullptr;
                assert(false);
            }
        }
    }

    vktrace_set_packet_entrypoint_end_time(pHeader);

    VkMemoryOpaqueCaptureAddressAllocateInfo captureAddressAllocateInfo = {};
    if (allocateFlagInfo != nullptr && (allocateFlagInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR)) {
        VkDeviceMemoryOpaqueCaptureAddressInfo memoryAddressInfo = {VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO, nullptr, *pMemory};
        uint64_t captureAddress = mdd(device)->devTable.GetDeviceMemoryOpaqueCaptureAddressKHR(device,&memoryAddressInfo);
        VkMemoryOpaqueCaptureAddressAllocateInfo *pCaptureAddressAllocateInfo = (VkMemoryOpaqueCaptureAddressAllocateInfo*)find_ext_struct((const vulkan_struct_header*)pAllocateInfo->pNext, VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO);
        if (pCaptureAddressAllocateInfo == nullptr) {
            pCaptureAddressAllocateInfo = (VkMemoryOpaqueCaptureAddressAllocateInfo*)find_ext_struct((const vulkan_struct_header*)pAllocateInfo->pNext, VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO_KHR);
        }
        if (pCaptureAddressAllocateInfo != nullptr) {
            pCaptureAddressAllocateInfo->opaqueCaptureAddress = captureAddress;
        } else {
            captureAddressAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO;
            captureAddressAllocateInfo.opaqueCaptureAddress = captureAddress;
            const void* temp = pAllocateInfo->pNext;
            const_cast<VkMemoryAllocateInfo*>(pAllocateInfo)->pNext = (const void*)&captureAddressAllocateInfo;
            captureAddressAllocateInfo.pNext = temp;
        }
    }

    pPacket = interpret_body_as_vkAllocateMemory(pHeader);
    pPacket->device = device;
    vktrace_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pAllocateInfo), sizeof(VkMemoryAllocateInfo), pAllocateInfo);
    if (pAllocateInfo) vktrace_add_pnext_structs_to_trace_packet(pHeader, (void*)pPacket->pAllocateInfo, pAllocateInfo);
    vktrace_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pAllocator), sizeof(VkAllocationCallbacks), NULL);
    vktrace_add_buffer_to_trace_packet(pHeader, (void**)&(pPacket->pMemory), sizeof(VkDeviceMemory), pMemory);
    pPacket->result = result;
    vktrace_finalize_buffer_address(pHeader, (void**)&(pPacket->pAllocateInfo));
    vktrace_finalize_buffer_address(pHeader, (void**)&(pPacket->pAllocator));
    vktrace_finalize_buffer_address(pHeader, (void**)&(pPacket->pMemory));

    if (!g_trimEnabled) {
        // trim not enabled, send packet as usual
        FINISH_TRACE_PACKET();
    } else {
        vktrace_finalize_trace_packet(pHeader);
        trim::ObjectInfo& info = trim::add_DeviceMemory_object(*pMemory);
        info.belongsToDevice = device;
        info.ObjectInfo.DeviceMemory.pCreatePacket = trim::copy_packet(pHeader);
        info.ObjectInfo.DeviceMemory.memoryTypeIndex = pAllocateInfo->memoryTypeIndex;
        info.ObjectInfo.DeviceMemory.propertyFlags = trim::LookUpMemoryProperties(device, pAllocateInfo->memoryTypeIndex);
        info.ObjectInfo.DeviceMemory.size = pAllocateInfo->allocationSize;
        VkMemoryDedicatedAllocateInfo *dedicatedAllocateInfo = (VkMemoryDedicatedAllocateInfo*)find_ext_struct((const vulkan_struct_header*)pAllocateInfo->pNext, VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
        if (dedicatedAllocateInfo != nullptr) {
            info.ObjectInfo.DeviceMemory.boundToBuffer = dedicatedAllocateInfo->buffer;
            info.ObjectInfo.DeviceMemory.boundToImage = dedicatedAllocateInfo->image;
        }
        if (pAllocator != NULL) {
            info.ObjectInfo.DeviceMemory.pAllocator = pAllocator;
            trim::add_Allocator(pAllocator);
        }
        if (g_trimIsInTrim) {
            trim::write_packet(pHeader);
        } else {
            vktrace_delete_trace_packet(&pHeader);
        }
    }

    // begin custom code
    add_new_handle_to_mem_info(*pMemory, pAllocateInfo->memoryTypeIndex, pAllocateInfo->allocationSize, NULL);
    // end custom code
    if (UseMappedExternalHostMemoryExtension()) {
        // only needed if user enable using VK_EXT_external_memory_host.
        pageguardExit();
    }
    return result;
}