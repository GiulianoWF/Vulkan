#include "interprocess_support.h"
#include "../../renderer/include/vertex_definition.h"

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

void PrintVertexPos(Vertex * v)
{
    std::cout << "{ " << v->pos.x << " , " << v->pos.y << " , " << v->pos.z << " } ";
}

void PrintVertexTexturePos(Vertex * v)
{
    std::cout <<  "{ " << v->texCoord.x << " , " << v->texCoord.y << " }"<< std::endl;
}

int main()
{
    InterprocessSupportMaster link;
    link.mCreateInterprocessLinks();

    void * data = link.mGetVertexDataPointer();
    Vertex * vertices_ptr = static_cast<Vertex *>(data);

    int64_t size = link.mGetVertexDataSize();
    size_t const count = size / sizeof(Vertex);
    
    std::vector<Vertex> vertices (vertices_ptr, vertices_ptr + count);

    for(int i = 0; i < count; ++i)
    {
        Vertex * current = &vertices_ptr[i];
        PrintVertexPos(current);
        PrintVertexTexturePos(current);

        current->texCoord.x = 0;
        current->texCoord.y = 0;
    }

    link.mMarkVertexForUpdate();
}