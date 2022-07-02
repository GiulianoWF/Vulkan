#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

struct Refresh {
    boost::interprocess::interprocess_mutex mutex;
    int refreshVertex = 0;
    int refreshIndex = 0;
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
        Refresh * mBufferRefreshInfo = nullptr;
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
            if(this->mBufferRefreshInfo->refreshIndex)
            {
                mUpdatePending |= (uint8_t)RefreshEnum::UPDATE_INDEX;
            }
            if(this->mBufferRefreshInfo->refreshVertex)
            {
                std::cout << "Must update Vertex" << std::endl;
                mUpdatePending |= (uint8_t)RefreshEnum::UPDATE_VERTEX;
            }
        }

        void mUpdateStagingBuffersInfo(uint64_t const vertexDataSize, void * vertexStagingData, uint64_t const indicesDataSize, void * indicesStagingData)
        {
            mVertexDataSize = vertexDataSize;
            mVertexStagingData = vertexStagingData;
            mIndicesDataSize = indicesDataSize;
            mIndicesStagingData = indicesStagingData;
        }

        void mUpdateStagingVertexBufferInfo(uint64_t const vertexDataSize, void * vertexStagingData)
        {
            mVertexDataSize = vertexDataSize;
            mVertexStagingData = vertexStagingData;
        }

        void mCreateInterprocessLinks(uint64_t const vertexDataSize, void * vertexStagingData, uint64_t const indicesDataSize, void * indicesStagingData)
        {
            //======================================================================================
            //                          Shared Memory Vertex
            //======================================================================================
            // VkDeviceSize dataSize = sizeof(vertices[0]) * vertices.size();
            this->mSharedMemoryObjectBuffer = bi::shared_memory_object(bi::open_or_create
                                                                    ,"VertexBuffer"
                                                                    ,bi::read_write );
            this->mSharedMemoryObjectBuffer.truncate(vertexDataSize);
            this->mMappedRegionBuffer = bi::mapped_region(this->mSharedMemoryObjectBuffer, bi::read_write);

            memcpy(this->mMappedRegionBuffer.get_address(), vertexStagingData, (size_t) vertexDataSize);
            // std::cout << "Verticies size " << vertexDataSize << std::endl;

            //======================================================================================
            //                          Shared Memory Index
            //======================================================================================
            // VkDeviceSize indexdataSize = sizeof(indices[0]) * indices.size();
            this->mSharedMemoryObjectIndex = bi::shared_memory_object(bi::open_or_create
                                                                    ,"IndexBuffer"
                                                                    ,bi::read_write
                                                                    );
            this->mSharedMemoryObjectIndex.truncate(indicesDataSize);
            this->mMappedRegionBufferIndex = bi::mapped_region(this->mSharedMemoryObjectIndex, bi::read_write);

            memcpy(this->mMappedRegionBufferIndex.get_address(), indicesStagingData, (size_t) indicesDataSize);

            //======================================================================================
            //                          Shared Memory Refresh
            //======================================================================================
            this->mSharedMemoryObjectRefresh = bi::shared_memory_object(bi::open_or_create
                                                                    ,"RefreshBuffer"
                                                                    ,bi::read_write
                                                                    );
            this->mSharedMemoryObjectRefresh.truncate(sizeof(Refresh));
            this->mMappedRegionBufferRefresh = bi::mapped_region(this->mSharedMemoryObjectRefresh, bi::read_write);

            this->mBufferRefreshInfo = new (this->mMappedRegionBufferRefresh.get_address()) Refresh;
            new (&this->mBufferRefreshInfo->mutex) boost::interprocess::interprocess_mutex{};

            memcpy(this->mMappedRegionBufferRefresh.get_address(), &this->mBufferRefreshInfo, (size_t) sizeof(Refresh));

            try{
                if (this->mBufferRefreshInfo->mutex.try_lock())
                {
                    std::cout << "Could not lock" << std::endl;
                }
                this->mBufferRefreshInfo->mutex.unlock();
            }
            catch(boost::interprocess::lock_exception& e)
            {
                std::cout << e.what();
                new (&this->mBufferRefreshInfo->mutex) boost::interprocess::interprocess_mutex{};
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
                } lock (&this->mBufferRefreshInfo->mutex);
                
                if(lock.getState())
                {
                    mRefreshUpdatePendingState();
                    // auto pending = this->updatePending();

                    if(mUpdatePending & (uint8_t)RefreshEnum::UPDATE_INDEX)
                    {
                        mUpdateIndexBuffer();
                        this->mBufferRefreshInfo->refreshIndex = 0;
                    }

                    if(mUpdatePending & (uint8_t)RefreshEnum::UPDATE_VERTEX)
                    {
                        mUpdateVertexBuffer();
                        this->mBufferRefreshInfo->refreshVertex = 0;
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
