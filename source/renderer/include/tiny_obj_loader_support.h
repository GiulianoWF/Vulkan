#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
struct VertexComposition {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const VertexComposition& other) const;
};

bool VertexComposition::operator==(const VertexComposition& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}

namespace std {
    template<> struct hash<VertexComposition> {
        size_t operator()(VertexComposition const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

class TinyObjLoaderSupport
{
    std::vector<VertexComposition> mVertices;
    std::vector<uint32_t> mIndices;

    protected:
        void mLoadModel(std::string const& modelPath)
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str())) {
                throw std::runtime_error(err);
            }

            std::unordered_map<VertexComposition, uint32_t> uniqueVertices{};

            for (const auto& shape : shapes) {
                for (const auto& index : shape.mesh.indices) {
                    VertexComposition vertex{};

                    vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };

                    vertex.color = {1.0f, 1.0f, 1.0f};

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
                        mVertices.push_back(vertex);
                    }

                    mIndices.push_back(uniqueVertices[vertex]);
                }
            }
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
