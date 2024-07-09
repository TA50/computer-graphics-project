#pragma once

#include "modules/Starter.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct NodeUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normal;
};

struct NodeVertex {
    alignas(16)  glm::vec3 pos;
    alignas(16)  glm::vec3 normal;
    alignas(16)  glm::vec2 uv;
    alignas(16)  glm::vec4 color;
};

class Node {
public:


    Node(std::string name) : name(name) {
        originalMatrix = glm::mat4(1.0f);
        transformedMatrix = glm::mat4(1.0f);
        originalMatrix = glm::mat4(1.0f);
        translation = glm::vec3(0);
        scale = glm::vec3(1);
        rotation = glm::quat(1, 0, 0, 0);
    }

    void init() {
// init pipelines and descriptor sets layouts
    }

// create pipeline and descriptor set
    void pipelinesAndDescriptorSetsInit() {}

    void pipelinesAndDescriptorSetsCleanup() {}

    void localCleanup() {}

    void bind(VkCommandBuffer commandBuffer, int currentImage) {}

    void dispatch(uint32_t currentImage) {}

    void update() {
        // apply local transformation
        // scale
        transformedMatrix = glm::scale(originalMatrix, scale);
        // rotate
        transformedMatrix = glm::mat4_cast(rotation) * transformedMatrix;
        // translate
        transformedMatrix = glm::translate(transformedMatrix, translation);
    }


    void setOriginalMatrix(glm::mat4 m) {
        originalMatrix = m;
        transformedMatrix = m;
    }

    void setTranslation(glm::vec3 t) {
        translation = t;
    }

    void setScale(glm::vec3 s) {
        scale = s;
    }

    void setRotation(glm::quat r) {
        rotation = r;
    }


    void addVertex(NodeVertex v) {
        vertices.push_back(v);
    }

    void addIndex(uint32_t i) {
        indices.push_back(i);
    }

    // Getters

    glm::mat4 getTransformedMatrix() {
        return transformedMatrix;
    }

    glm::mat4 getOriginalMatrix() {
        return originalMatrix;
    }

    std::vector<NodeVertex> getVertices() {
        return vertices;
    }

    std::vector<uint32_t> getIndices() {
        return indices;
    }

    std::string getName() {
        return name;
    }


protected:
    VertexDescriptor VD;
    Pipeline P;
    DescriptorSet DS;
    DescriptorSetLayout DSL;


    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<NodeVertex> vertices;
    std::vector<uint32_t> indices;


    glm::vec3 translation;
    glm::vec3 scale;
    glm::quat rotation;

    glm::mat4 originalMatrix;
    glm::mat4 transformedMatrix;

    tinygltf::Node rawNode;
    std::string name;


};