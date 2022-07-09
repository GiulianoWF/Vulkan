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

class VertexManipulator
{
    Vertex * m_cVertices;
    size_t const m_iVertexCount;

    InterprocessSupportMaster & m_cLink;

public:
    VertexManipulator(Vertex * verticies, size_t const vertexCount, InterprocessSupportMaster & link)
        : m_cVertices(verticies)
        , m_iVertexCount(vertexCount)
        , m_cLink(link)
    {};

    [[noreturn]]
    void Loop()
    {
        std::string input;
        while(true){
            std::cout << "\n$ ";
            std::cin >> input;

            if(input == "help")
            {
                std::cout << "color_all   -   Collor all verticies with a color." << std::endl;
                std::cout << "move_vertex   -   Select and move a vertex" << std::endl;
            }
            else if(input == "color_all")
            {
                ColorAllVerticies();
            }
            else if(input == "move_vertex")
            {
                MoveVertice();
            }
        }
    }

private:
    void ColorAllVerticies()
    {
        float r, g, b;
        std::cout << "Enter R" << std::endl;
        std::cin >> r;
        std::cout << "Enter G" << std::endl;
        std::cin >> g;
        std::cout << "Enter B" << std::endl;
        std::cin >> b;

        for(size_t i = 0; i < m_iVertexCount; ++i)
        {
            Vertex & current = m_cVertices[i];
            current.color = {r,g,b};
        }

        m_cLink.mMarkVertexForUpdate();
    }

    void MoveVertice()
    {
        std::cout << "Index? $";
        size_t index;
        std::cin >> index;

        Vertex & current = m_cVertices[index];

        std::string userInput("");
        while(userInput != "exit")
        {
            std::cout << "\nwasdqe  $";
            std::cin >> userInput;
            if(userInput == "q")
            {
                current.pos.x += 0.1;
            }
            else if(userInput == "e")
            {
                current.pos.x -= 0.1;
            }
            else if(userInput == "w")
            {
                current.pos.z += 0.1;
            }
            else if(userInput == "s")
            {
                current.pos.z -= 0.1;
            }
            else if(userInput == "a")
            {
                current.pos.y += 0.1;
            }
            else if(userInput == "d")
            {
                current.pos.y -= 0.1;
            }
            else
            {
                continue;
            }
            m_cLink.mMarkVertexForUpdate();
        }
    }
};

int main()
{
    InterprocessSupportMaster link;
    link.mCreateInterprocessLinks();

    void * data = link.mGetVertexDataPointer();
    Vertex * vertices_ptr = static_cast<Vertex *>(data);

    int64_t size = link.mGetVertexDataSize();
    size_t const count = size / sizeof(Vertex);

    VertexManipulator vm(vertices_ptr, count, link);

    vm.Loop();
}
