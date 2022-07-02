#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

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
};

struct Refresh {
    boost::interprocess::interprocess_mutex mutex;
    int refreshVertex = 0;
    int refreshIndex = 0;
};

int main()
{
    std::vector<Vertex> vertices;
    vertices.reserve(114112);

    boost::interprocess::shared_memory_object mSharedMemoryObject;
    boost::interprocess::mapped_region mMappedRegion;

    mSharedMemoryObject = bi::shared_memory_object(bi::open_only
                                                    ,"VertexBuffer"
                                                    ,bi::read_write
                                                    );

    mMappedRegion = bi::mapped_region(mSharedMemoryObject, bi::read_write);

    int a ;
    std::cin >> a;

    Vertex * data = (Vertex*)mMappedRegion.get_address();
    for(int i =0; i < 114112; ++i)
    {
        std::cout << (data+i)->pos.x << std::endl;
        (data+i)->pos.x = 0;
        std::cout << (data+i)->pos.x << std::endl;
    }

    boost::interprocess::shared_memory_object refreshObject;
    boost::interprocess::mapped_region refreshRegion;

    refreshObject = bi::shared_memory_object(bi::open_only
                                            ,"RefreshBuffer"
                                            ,bi::read_write
                                            );

    refreshRegion = bi::mapped_region(refreshObject, bi::read_write);

    Refresh * refreshInfo = (Refresh*)refreshRegion.get_address();

    refreshInfo->refreshVertex = 1;
}