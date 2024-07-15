#pragma once

#include "game-object.hpp"
#include <string>
#include <iostream>
#include <vector>


class GameObjectLoader {
public:
    struct GameObjectLoaderResult {
        std::vector<GameObjectVertex> vertices;
        std::vector<uint32_t> indices;
        glm::mat4 Wm;
    };

    static GameObjectLoaderResult loadModelOBJ(std::string file) {

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        GameObjectLoaderResult result{};
        result.Wm = glm::mat4(1.0f);
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              file.c_str())) {
            std::cout << "Error loading GLTF file: " << file << "\n";
            throw std::runtime_error(warn + err);
        }


        for (const auto &shape: shapes) {
            for (const auto &index: shape.mesh.indices) {
                GameObjectVertex vertex{};
                glm::vec3 pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.setColor({
                                        attrib.colors[3 * index.vertex_index + 0],
                                        attrib.colors[3 * index.vertex_index + 1],
                                        attrib.colors[3 * index.vertex_index + 2],
                                        1.0f
                                });

                vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1 - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };

                result.vertices.push_back(vertex);
                result.indices.push_back(result.vertices.size() - 1);
            }
        }
        std::cout << "[OBJ] Vertices: " << result.vertices.size() << "\n";
        std::cout << "Indices: " << result.indices.size() << "\n";

        return result;
    }

    static GameObjectLoaderResult loadGltf(std::string file) {

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string warn, err;

        GameObjectLoaderResult result{};


        if (!loader.LoadASCIIFromFile(&model, &warn, &err,
                                      file.c_str())) {
            std::cout << "Error loading GLTF file: " << file << "\n";
            throw std::runtime_error(warn + err);
        }


        for (const auto &mesh: model.meshes) {
            for (const auto &primitive: mesh.primitives) {
                if (primitive.indices < 0) {
                    continue;
                }

                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;
                const float *bufferTangents = nullptr;
                const float *bufferTexCoords = nullptr;

                bool meshHasPos = false;
                bool meshHasNorm = false;
                bool meshHasTan = false;
                bool meshHasUV = false;

                int cntPos = 0;
                int cntNorm = 0;
                int cntTan = 0;
                int cntUV = 0;
                int cntTot = 0;

                auto pIt = primitive.attributes.find("POSITION");
                if (pIt != primitive.attributes.end()) {
                    const tinygltf::Accessor &posAccessor = model.accessors[pIt->second];
                    const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
                    bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[
                            posAccessor.byteOffset + posView.byteOffset]));
                    meshHasPos = true;
                    cntPos = posAccessor.count;
                    if (cntPos > cntTot) cntTot = cntPos;
                } else {
                    std::cout << "Error: No position attribute found\n";
                    exit(EXIT_FAILURE);
                }

                auto nIt = primitive.attributes.find("NORMAL");
                if (nIt != primitive.attributes.end()) {
                    const tinygltf::Accessor &normAccessor = model.accessors[nIt->second];
                    const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
                    bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[
                            normAccessor.byteOffset + normView.byteOffset]));
                    meshHasNorm = true;
                    cntNorm = normAccessor.count;
                    if (cntNorm > cntTot) cntTot = cntNorm;
                } else {

                    std::cout << "Warning: vertex layout has normal, but file hasn't\n";
                    exit(EXIT_FAILURE);
                }

                auto uIt = primitive.attributes.find("TEXCOORD_0");
                if (uIt != primitive.attributes.end()) {
                    const tinygltf::Accessor &uvAccessor = model.accessors[uIt->second];
                    const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
                    bufferTexCoords = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[
                            uvAccessor.byteOffset + uvView.byteOffset]));
                    meshHasUV = true;
                    cntUV = uvAccessor.count;
                    if (cntUV > cntTot) cntTot = cntUV;
                } else {
                    std::cout << "Warning: vertex layout has UV, but file hasn't\n";
                    exit(EXIT_FAILURE);
                }


                for (int i = 0; i < cntTot; i++) {
                    GameObjectVertex vertex{};

                    if ((i < cntPos) && meshHasPos) {
                        vertex.pos = {
                                bufferPos[3 * i + 0],
                                bufferPos[3 * i + 1],
                                bufferPos[3 * i + 2]
                        };


                    }
                    if ((i < cntNorm) && meshHasNorm) {
                        vertex.normal = {
                                bufferNormals[3 * i + 0],
                                bufferNormals[3 * i + 1],
                                bufferNormals[3 * i + 2]
                        };
                    }

                    if ((i < cntUV) && meshHasUV) {
                        vertex.uv = {
                                bufferTexCoords[2 * i + 0],
                                bufferTexCoords[2 * i + 1]
                        };
                    }

                    result.vertices.push_back(vertex);

                }

                const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        const uint16_t *bufferIndex = reinterpret_cast<const uint16_t *>(&(buffer.data[
                                accessor.byteOffset + bufferView.byteOffset]));
                        for (int i = 0; i < accessor.count; i++) {
                            result.indices.push_back(bufferIndex[i]);
                        }
                    }
                        break;
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        const uint32_t *bufferIndex = reinterpret_cast<const uint32_t *>(&(buffer.data[
                                accessor.byteOffset + bufferView.byteOffset]));
                        for (int i = 0; i < accessor.count; i++) {
                            result.indices.push_back(bufferIndex[i]);
                        }
                    }
                        break;
                    default:
                        throw std::runtime_error("Error loading GLTF component");
                }
            }
        }

        glm::vec3 T;
        glm::vec3 S;
        glm::quat Q;
        if (model.nodes[0].translation.size() > 0) {

            T = glm::vec3(model.nodes[0].translation[0],
                          model.nodes[0].translation[1],
                          model.nodes[0].translation[2]);
        } else {
            T = glm::vec3(0);
        }
        if (model.nodes[0].rotation.size() > 0) {
//std::cout << "node " << i << " has Q\n";
            Q = glm::quat(model.nodes[0].rotation[3],
                          model.nodes[0].rotation[0],
                          model.nodes[0].rotation[1],
                          model.nodes[0].rotation[2]);
        } else {
            Q = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        if (model.nodes[0].scale.size() > 0) {

            S = glm::vec3(model.nodes[0].scale[0],
                          model.nodes[0].scale[1],
                          model.nodes[0].scale[2]);
        } else {
            S = glm::vec3(1);
        }

        result.Wm = glm::translate(glm::mat4(1), T) *
                    glm::mat4(Q) *
                    glm::scale(glm::mat4(1), S);

        return result;
    }

};