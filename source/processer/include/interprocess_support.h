#pragma once

#include "../../renderer/include/vertex_definition.h"

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <iostream>

namespace bi = boost::interprocess;

struct LinkControl {
    boost::interprocess::interprocess_mutex mutex;
    int refreshVertex = 0;
    int refreshIndex = 0;
    size_t mVertexDataSize = 0;
    size_t mIndicesDataSize = 0;
};

enum struct RefreshEnum : uint8_t
{
    UPDATE_VERTEX = 1 << 0,
    UPDATE_INDEX = 1 << 1,
};

class InterprocessSupportMaster
{
        boost::interprocess::shared_memory_object mSharedMemoryObjectBuffer;
        boost::interprocess::mapped_region mMappedRegionBuffer;
        boost::interprocess::shared_memory_object mSharedMemoryObjectIndex;
        boost::interprocess::mapped_region mMappedRegionBufferIndex;
        boost::interprocess::shared_memory_object mSharedMemoryObjectRefresh;
        boost::interprocess::mapped_region mMappedRegionBufferRefresh;

        uint64_t mVertexDataSize = 0;
        void * mVertexStagingData = nullptr;
        uint64_t mIndicesDataSize = 0;
        void * mIndicesStagingData = nullptr;
        LinkControl * mLinkControlInfo = nullptr;
        uint8_t mUpdatePending = 0;

    public:
        auto mGetVertexDataPointer() -> void *
        {
            return this->mVertexStagingData;
        }

        auto mGetVertexDataSize() -> uint64_t
        {
            return this->mLinkControlInfo->mVertexDataSize;
        }

        void mCreateInterprocessLinks()
        {
            //======================================================================================
            //                          Shared Memory Vertex
            //======================================================================================

            this->mSharedMemoryObjectBuffer = bi::shared_memory_object(bi::open_only
                                                                    ,"VertexBuffer"
                                                                    ,bi::read_write );
            this->mMappedRegionBuffer = bi::mapped_region(this->mSharedMemoryObjectBuffer, bi::read_write);
            this->mVertexStagingData = this->mMappedRegionBuffer.get_address();

            //======================================================================================
            //                          Shared Memory Index
            //======================================================================================

            this->mSharedMemoryObjectIndex = bi::shared_memory_object(bi::open_only
                                                                    ,"IndexBuffer"
                                                                    ,bi::read_write
                                                                    );

            this->mMappedRegionBufferIndex = bi::mapped_region(this->mSharedMemoryObjectIndex, bi::read_write);
            this->mIndicesStagingData = this->mMappedRegionBufferIndex.get_address();

            //======================================================================================
            //                          Shared Memory Refresh
            //======================================================================================
            this->mSharedMemoryObjectRefresh = bi::shared_memory_object(bi::open_or_create
                                                                    ,"RefreshBuffer"
                                                                    ,bi::read_write
                                                                    );

            this->mMappedRegionBufferRefresh = bi::mapped_region(this->mSharedMemoryObjectRefresh, bi::read_write);
            void * refreshData_ptr = this->mMappedRegionBufferRefresh.get_address();
            this->mLinkControlInfo = static_cast<LinkControl *>(refreshData_ptr);
        }

        void mMarkVertexForUpdate()
        {
            try
            {
                struct Lock{
                    boost::interprocess::interprocess_mutex * ptr;
                    bool lockAquired = false;

                    Lock(boost::interprocess::interprocess_mutex * p):ptr(p)
                    {
                        lockAquired = ptr->try_lock();
                    };

                    ~Lock()
                    {
                        if(lockAquired) ptr->unlock();
                    };

                    bool getState()
                    {
                        return lockAquired;
                    };
                } lock (&this->mLinkControlInfo->mutex);

                if(lock.getState())
                {
                    this->mLinkControlInfo->refreshVertex = 1;
                }
            }
            catch (boost::interprocess::lock_exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
};
