#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
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

class InterprocessSupport
{
        boost::interprocess::shared_memory_object mSharedMemoryObjectBuffer;
        boost::interprocess::mapped_region mMappedRegionBuffer;
        boost::interprocess::shared_memory_object mSharedMemoryObjectIndex;
        boost::interprocess::mapped_region mMappedRegionBufferIndex;
        boost::interprocess::shared_memory_object mSharedMemoryObjectRefresh;
        boost::interprocess::mapped_region mMappedRegionBufferRefresh;
        LinkControl * mLinkControlInfo = nullptr;
        uint8_t mUpdatePending = 0;

        uint64_t mVertexDataSize = 0;
        void * mVertexStagingData = nullptr;
        uint64_t mIndicesDataSize = 0;
        void * mIndicesStagingData = nullptr;

    protected:
        bool mVertexDownloadPending = 0;

        void mRefreshUpdatePendingState() 
        {
            mUpdatePending = 0;
            if(this->mLinkControlInfo->refreshIndex)
            {
                mUpdatePending |= (uint8_t)RefreshEnum::UPDATE_INDEX;
            }
            if(this->mLinkControlInfo->refreshVertex)
            {
                mUpdatePending |= (uint8_t)RefreshEnum::UPDATE_VERTEX;
            }
        }

        void mUpdateStagingBuffersInfo(uint64_t const vertexDataSize, void * vertexStagingData, uint64_t const indicesDataSize, void * indicesStagingData)
        {
            mVertexDataSize = vertexDataSize;
            if (nullptr != this->mLinkControlInfo)
            {
                this->mLinkControlInfo->mVertexDataSize = vertexDataSize;
            }
            mVertexStagingData = vertexStagingData;

            mIndicesDataSize = indicesDataSize;
            if (nullptr != this->mLinkControlInfo)
            {
                this->mLinkControlInfo->mIndicesDataSize = indicesDataSize;
            }
            mIndicesStagingData = indicesStagingData;
        }

        void mUpdateStagingVertexBufferInfo(uint64_t const vertexDataSize, void * vertexStagingData)
        {
            mVertexDataSize = vertexDataSize;
            if (nullptr != this->mLinkControlInfo)
            {
                this->mLinkControlInfo->mVertexDataSize = vertexDataSize;
            }
            mVertexStagingData = vertexStagingData;
        }

        void mCreateInterprocessLinks(uint64_t const vertexDataSize, void * vertexStagingData, uint64_t const indicesDataSize, void * indicesStagingData)
        {
            //======================================================================================
            //                          Shared Memory Refresh
            //======================================================================================
            this->mSharedMemoryObjectRefresh = bi::shared_memory_object(bi::open_or_create
                                                                    ,"RefreshBuffer"
                                                                    ,bi::read_write
                                                                    );
            this->mSharedMemoryObjectRefresh.truncate(sizeof(LinkControl));
            this->mMappedRegionBufferRefresh = bi::mapped_region(this->mSharedMemoryObjectRefresh, bi::read_write);

            this->mLinkControlInfo = new (this->mMappedRegionBufferRefresh.get_address()) LinkControl;
            new (&this->mLinkControlInfo->mutex) boost::interprocess::interprocess_mutex{};

            memcpy(this->mMappedRegionBufferRefresh.get_address(), &this->mLinkControlInfo, (size_t) sizeof(LinkControl));

            try{
                if (this->mLinkControlInfo->mutex.try_lock())
                {
                    std::cout << "Could not lock" << std::endl;
                }
                this->mLinkControlInfo->mutex.unlock();
            }
            catch(boost::interprocess::lock_exception& e)
            {
                std::cout << e.what();
                new (&this->mLinkControlInfo->mutex) boost::interprocess::interprocess_mutex{};
            }

            //======================================================================================
            //                          Shared Memory Vertex
            //======================================================================================
            this->mSharedMemoryObjectBuffer = bi::shared_memory_object(bi::open_or_create
                                                                    ,"VertexBuffer"
                                                                    ,bi::read_write );

            this->mSharedMemoryObjectBuffer.truncate(vertexDataSize);
            this->mLinkControlInfo->mVertexDataSize = vertexDataSize;

            this->mMappedRegionBuffer = bi::mapped_region(this->mSharedMemoryObjectBuffer, bi::read_write);

            if(nullptr != vertexStagingData && nullptr != this->mMappedRegionBuffer.get_address())
            {
                memcpy(this->mMappedRegionBuffer.get_address(), vertexStagingData, (size_t) vertexDataSize);            // std::cout << "Verticies size " << vertexDataSize << std::endl;
            }

            //======================================================================================
            //                          Shared Memory Index
            //======================================================================================
            this->mSharedMemoryObjectIndex = bi::shared_memory_object(bi::open_or_create
                                                                    ,"IndexBuffer"
                                                                    ,bi::read_write
                                                                    );

            this->mSharedMemoryObjectIndex.truncate(indicesDataSize);
            this->mLinkControlInfo->mIndicesDataSize = indicesDataSize;

            this->mMappedRegionBufferIndex = bi::mapped_region(this->mSharedMemoryObjectIndex, bi::read_write);

            if(nullptr != indicesStagingData && nullptr != this->mMappedRegionBufferIndex.get_address())
            {
                memcpy(this->mMappedRegionBufferIndex.get_address(), indicesStagingData, (size_t) indicesDataSize);
            }
        }

        void mInterprocessUpdateStagingBuffersIfNeeded()
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
                    mRefreshUpdatePendingState();
                    // auto pending = this->updatePending();

                    if(mUpdatePending & (uint8_t)RefreshEnum::UPDATE_INDEX)
                    {
                        mUpdateIndexBuffer();
                        this->mLinkControlInfo->refreshIndex = 0;
                    }

                    if(mUpdatePending & (uint8_t)RefreshEnum::UPDATE_VERTEX)
                    {
                        mUpdateVertexBuffer();
                        this->mLinkControlInfo->refreshVertex = 0;
                    }
                }
                else
                {
                    std::cout << "Have not get refreash lock." << std::endl;
                }
            }
            catch (boost::interprocess::lock_exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }

        void mUpdateVertexBuffer()
        {
            std::cout << "Updating Buffer ";

            std::cout << (long int)mVertexStagingData << std::endl;

            // VkDeviceSize dataSize = sizeof(vertices[0]) * vertices.size();
            memcpy(mVertexStagingData, this->mMappedRegionBuffer.get_address(), (size_t) mVertexDataSize);
            mVertexDownloadPending = true;
        }

        void mUpdateIndexBuffer()
        {

        }
};
