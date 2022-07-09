#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "vertex_definition.h"

#include <iostream>

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

class TinyObjLoaderSupport
{
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;

    protected:
        void mLoadModel(std::string const& modelPath)
        {
            Vertex vertex0{
                .pos{0.2f, -0.2f, -0.2f},
                .color{0.0f, 0.0f, 1.0f},
                .texCoord{0.375f, -0.167599}
            };
            Vertex vertex1{
                .pos{0.2f, 0.2f, -0.2f},
                .color{0.0f, 1.0f, 0.0f},
                .texCoord{0.625f, 0.232401}
            };
            Vertex vertex2{
                .pos{0.2, 0.2f, 0.2f},
                .color{1.0f, 0.0f, 0.0f},
                .texCoord{0.625f, 0.232401}
            };
            Vertex vertex3{
                .pos{0.2f, -0.2f, 0.2f},
                .color{0.4f, 0.4f, 0.4f},
                .texCoord{0.625f, 0.232401}
            };

            Vertex vertex4{
                .pos{-0.2f, -0.2f, -0.2f},
                .color{0.0f, 0.0f, 1.0f},
                .texCoord{0.375f, -0.167599}
            };
            Vertex vertex5{
                .pos{-0.2f, 0.2f, -0.2f},
                .color{0.0f, 1.0f, 0.0f},
                .texCoord{0.375f, -0.167599}
            };
            Vertex vertex6{
                .pos{-0.2, 0.2f, 0.2f},
                .color{1.0f, 0.0f, 0.0f},
                .texCoord{0.625f, 0.848625}
            };
            Vertex vertex7{
                .pos{-0.2f, -0.2f, 0.2f},
                .color{0.4f, 0.4f, 0.4f},
                .texCoord{0.625f, 0.848625}
            };
            Vertex vertex8{
                .pos{-0.2, 0.2f, 0.2f},
                .color{0.1f, 0.848625f, 0.165054f},
                .texCoord{0.625f, 0.848625}
            };
            mVertices.push_back(vertex0);
            mVertices.push_back(vertex1);
            mVertices.push_back(vertex2);
            mVertices.push_back(vertex3);
            mVertices.push_back(vertex4);
            mVertices.push_back(vertex5);
            mVertices.push_back(vertex6);
            mVertices.push_back(vertex7);
            mVertices.push_back(vertex8);

        //=================================
            mIndices.push_back(0);
            mIndices.push_back(1);
            mIndices.push_back(2);

            mIndices.push_back(0);
            mIndices.push_back(2);
            mIndices.push_back(3);

        //=================================
            mIndices.push_back(5);
            mIndices.push_back(4);
            mIndices.push_back(6);

            mIndices.push_back(6);
            mIndices.push_back(4);
            mIndices.push_back(7);

        //=================================
            mIndices.push_back(0);
            mIndices.push_back(1);
            mIndices.push_back(5);

            mIndices.push_back(0);
            mIndices.push_back(5);
            mIndices.push_back(4);

        //=================================
            mIndices.push_back(3);
            mIndices.push_back(2);
            mIndices.push_back(6);

            mIndices.push_back(3);
            mIndices.push_back(6);
            mIndices.push_back(7);

        //=================================
            mIndices.push_back(5);
            mIndices.push_back(6);
            mIndices.push_back(2);

            mIndices.push_back(5);
            mIndices.push_back(2);
            mIndices.push_back(1);

        //=================================
            mIndices.push_back(7);
            mIndices.push_back(4);
            mIndices.push_back(3);

            mIndices.push_back(0);
            mIndices.push_back(3);
            mIndices.push_back(4);
        }

        void mCopyModelVertexDataTo(void * stagingVertexData)
        {
            memcpy(stagingVertexData, mVertices.data(), mGetModelVertexDataSize());
        }

        void mCopyModelIndicesDataTo(void * stagingIndicesData)
        {
            memcpy(stagingIndicesData, mIndices.data(), mGetModelIndicesDataSize());
        }

        inline size_t mGetModelVertexDataSize()
        {
            size_t static const dataSizeUnit = sizeof(mVertices[0]);
            auto const dataSize = dataSizeUnit * mVertices.size();

            return dataSize;
        }

        inline size_t mGetModelIndicesDataSize()
        {
            size_t static const dataSizeUnit = sizeof(mIndices[0]);
            auto const dataSize = dataSizeUnit * mIndices.size();

            return dataSize;
        }

        inline uint32_t mGetIndiciesCount()
        {
            return static_cast<uint32_t>(mIndices.size());
        }
};
