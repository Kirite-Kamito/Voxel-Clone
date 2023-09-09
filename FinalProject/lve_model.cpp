#include "lve_model.hpp"

#include "lve_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#define VOXELIZER_IMPLEMENTATION
#include "voxelizer/voxelizer.h"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>


namespace std {
    template <>
    struct hash<lve::LveModel::Vertex> {
        size_t operator()(lve::LveModel::Vertex const& vertex) const {
            size_t seed = 0;
            lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std

namespace lve {

    LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder) : lveDevice{ device } {
        createVertexBuffers(builder.vertices);
        createIndexBuffer(builder.indices);
        
    }

    LveModel::~LveModel() {
        vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

        if (hasIndexBuffer) {
            vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
            vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
        }
    }

    std::unique_ptr<LveModel> LveModel::createModelFromFile(
        LveDevice& device, const std::string& filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_unique<LveModel>(device, builder);
    }

    std::unique_ptr<LveModel> LveModel::createVoxelFromFile(
        LveDevice& device, glm::vec3 color) {
        Builder builder{};
        builder.loadVoxel(color);
        return std::make_unique<LveModel>(device, builder);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBuffer,
            vertexBufferMemory);

        lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
    }

    void LveModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        lveDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indexBuffer,
            indexBufferMemory);

        lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
    }

    void LveModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void LveModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });


        return attributeDescriptions;
    }

    void LveModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        bool useVoxel = false;

        std::cout << "Attempt Voxelizer? (likely to fail) (1 for yes, 0 for no): ";
        std::cin >> useVoxel;



        if (useVoxel) {
            //int totalVertices = 0;
            //int vertCounter = 0;
            for (int i = 0; i < shapes.size(); i++) {
                vx_mesh_t* mesh;
                vx_mesh_t* result;
                mesh = vx_color_mesh_alloc(attrib.vertices.size(), shapes[i].mesh.indices.size());

                for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++) {
                    mesh->indices[f] = shapes[i].mesh.indices[f].vertex_index;
                }

                for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
                    mesh->vertices[v].x = attrib.vertices[3 * v + 0];
                    mesh->vertices[v].y = attrib.vertices[3 * v + 1];
                    mesh->vertices[v].z = attrib.vertices[3 * v + 2];

                    mesh->colors[v].r = attrib.colors[3 * v + 0];
                    mesh->colors[v].g = attrib.colors[3 * v + 1];
                    mesh->colors[v].b = attrib.colors[3 * v + 2];
                }

                for (size_t n = 0; n < attrib.normals.size() / 3; n++) {
                    mesh->normals[n].x = attrib.normals[3 * n + 0];
                    mesh->normals[n].y = attrib.normals[3 * n + 1];
                    mesh->normals[n].z = attrib.normals[3 * n + 2];
                }

                result = vx_voxelize(mesh, .1, .1, .1, .05f);

                for (int v = 0; v < result->nvertices; v++) {
                    Vertex vertex{};
                    vertex.position = {
                        result->vertices[v].x,
                        result->vertices[v].y,
                        result->vertices[v].z
                    };
                    vertex.color = {
                        result->colors->r,
                        result->colors->g,
                        result->colors->b,
                    };

                    vertices.push_back(vertex);
                    //vertCounter++;
                    /*
                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                    */
                }
                /*
                for (int ind = 0; i < result->nindices; i++) {
                    indices.push_back(result->indices[ind]+totalVertices);
                }
                
                totalVertices += vertCounter;
                vertCounter = 0;
                */
                vx_mesh_free(result);
                vx_mesh_free(mesh);
            }
        }
        else {
            for (const auto& shape : shapes) {
                for (const auto& index : shape.mesh.indices) {
                    Vertex vertex{};

                    if (index.vertex_index >= 0) {
                        vertex.position = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2],
                        };

                        vertex.color = {
                            attrib.colors[3 * index.vertex_index + 0],
                            attrib.colors[3 * index.vertex_index + 1],
                            attrib.colors[3 * index.vertex_index + 2],
                        };
                    }

                    if (index.normal_index >= 0) {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2],
                        };
                    }

                    if (index.texcoord_index >= 0) {
                        vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1],
                        };
                    }

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                }
            }
        }
    }

    void LveModel::Builder::loadVoxel(glm::vec3 color) {
        const std::string filepath = "./models/voxel.obj";
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0] * color.x,
                        attrib.colors[3 * index.vertex_index + 1] * color.y,
                        attrib.colors[3 * index.vertex_index + 2] * color.z,
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}  // namespace lve