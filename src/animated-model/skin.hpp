#pragma once

#include <string>
#include <vector>
#include "animated-model/joint.hpp"
#include "modules/Starter.hpp"
#include "animated-model/joint.hpp"
#include "camera.hpp"

#define MAX_JOINTS_COUNT 100

struct SkinVertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 jointIndices;
    glm::vec4 jointWeights;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(SkinVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      pos)),          sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      normal)),       sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      uv)),           sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      jointIndices)), sizeof(glm::vec4), OTHER},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      jointWeights)), sizeof(glm::vec4), OTHER}
        };
    }
};

struct SkinMvpObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 jointTransformMatrices[MAX_JOINTS_COUNT];
};

struct SkinInverseBindMatrixObject {
    alignas(16) glm::mat4 inverseBindMatrices[MAX_JOINTS_COUNT];
};

class Skin {
public:
    explicit Skin(std::string name, int jointsCount) : name(std::move(name)), jointsCount(jointsCount) {
        rootJointIndex = -1;

        joints = std::unordered_map<int, Joint *>();
        inverseBindMatrices = std::unordered_map<int, glm::mat4>();
        jointMatrices = std::unordered_map<int, glm::mat4>();

        vertices = std::vector<SkinVertex>();
        indices = std::vector<uint32_t>();

        BP = nullptr;
//        P = Pipeline();
//        VD = VertexDescriptor();
//        DSL = DescriptorSetLayout();
//        DS = DescriptorSet();


    }

    static PoolSizes getPoolSizes() {
        PoolSizes DPSZs = {};
        DPSZs.uniformBlocksInPool = 2;
        DPSZs.texturesInPool = 0;
        DPSZs.setsInPool = 2;
        return DPSZs;
    }

    void init(BaseProject *bp, Camera *_camera) {
        BP = bp;
        camera = _camera;
        VD.init(bp, SkinVertex::getBindingDescription(), SkinVertex::getDescriptorElements());
        DSL.init(bp, {
                {Mvp_BINDING,               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SkinMvpObject),               1},
                {InverseBindMatrix_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SkinInverseBindMatrixObject), 1}
        });

        P.init(bp, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              VK_CULL_MODE_BACK_BIT, false);
        createVertexBuffer();
        createIndexBuffer();


    }

    void createPipelineAndDescriptorSets() {
        P.create();
        std::cout << "[Skin]: Pipeline created\n";
        DS.init(BP, &DSL, {});
        std::cout << "[Skin]: Descriptor Set Created\n";
    }

    void bind(VkCommandBuffer commandBuffer, int currentImage) {
        P.bind(commandBuffer);
        DS.bind(commandBuffer, P, InverseBinding_Mvp_SET, currentImage);

        //vertex and index buffers

        VkBuffer vertexBuffers[] = {vertexBuffer};
        // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        SkinInverseBindMatrixObject inverseBindMatrixObject{};
        for (auto kv: inverseBindMatrices) {
            inverseBindMatrixObject.inverseBindMatrices[kv.first] = kv.second;
        }

        DS.map(currentImage, &inverseBindMatrixObject, InverseBindMatrix_BINDING);
    }

    void updateUniformBuffers(uint32_t currentImage) {
        std::cout << "Updating uniform buffers\n";
        updateJointMatrices();
        SkinMvpObject mvpObject{};
        mvpObject.model = getModelMatrix();
        mvpObject.view = camera->matrices.view;
        mvpObject.projection = camera->matrices.perspective;

        DS.map(currentImage, &mvpObject, Mvp_BINDING);
    }

    void pipelinesAndDescriptorSetsCleanup() {
        DS.cleanup();
        P.cleanup();
    }

    void localCleanup() {
        P.destroy();
        VD.cleanup();
        DSL.cleanup();

        vkDestroyBuffer(BP->device, vertexBuffer, nullptr);
        vkFreeMemory(BP->device, vertexBufferMemory, nullptr);

        vkDestroyBuffer(BP->device, indexBuffer, nullptr);
        vkFreeMemory(BP->device, indexBufferMemory, nullptr);
    }

    void updateJointMatrices() {
        // copy transforms from joints to jointMatrices
        for (auto kv: joints) {
            kv.second->updateTransformedMatrix();
            jointMatrices[kv.first] = kv.second->getTransformedMatrix();
        }
    }


    void addJoint(Joint *joint, glm::mat4 inverseBindMatrix, int jointIndex) {
        joints[jointIndex] = joint;
        inverseBindMatrices[jointIndex] = inverseBindMatrix;
        joint->updateTransformedMatrix();
        jointMatrices[jointIndex] = joint->getTransformedMatrix();
        joints[jointIndex] = joint;
    }

    void addVertex(SkinVertex vertex) {
        vertices.push_back(vertex);
    }

    void addIndex(uint32_t index) {
        indices.push_back(index);
    }



    // Setters

    void setRootJoint(int index) {
        rootJointIndex = index;
    }


    // Getters
    Joint *getJoint(int index) {
        return joints[index];
    }

    glm::mat4 getInverseBindMatrix(int index) {
        return inverseBindMatrices[index];
    }

    glm::mat4 getJointMatrix(int index) {
        return jointMatrices[index];
    }

    int getRootJointIndex() const {
        return rootJointIndex;
    }

    std::string getName() {
        return name;
    }

    size_t getJointCount() {
        return joints.size();
    }

    std::vector<int> getJointIndices() {
        std::vector<int> jointIndices;
        for (auto kv: joints) {
            jointIndices.push_back(kv.first);
        }
        return jointIndices;
    }

    std::vector<uint32_t> getVertexIndices() {
        return indices;
    }

    std::vector<SkinVertex> getVertices() {
        std::vector<int> normalIndices;
        return vertices;
    }

    glm::mat4 getModelMatrix() {
        if (rootJointIndex >= 0 && joints.find(rootJointIndex) != joints.end()) {
            return joints[rootJointIndex]->getGlobalMatrix();
        }
        return glm::mat4(1.0f);
    }

protected:
    int rootJointIndex;
    int jointsCount;
    std::string name;
    std::unordered_map<int, Joint *> joints;
    std::vector<SkinVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<int, glm::mat4> inverseBindMatrices;
    std::unordered_map<int, glm::mat4> jointMatrices{};


    DescriptorSetLayout DSL;
    DescriptorSet DS;

    DescriptorSetLayout InverseBindMatrixDSL;
    DescriptorSet InverseBindMatrixDS;
    Pipeline P;
    VertexDescriptor VD;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;


    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;


    BaseProject *BP;
    Camera *camera;

    const std::string VERT_SHADER = "assets/shaders/bin/skinning.vert.spv";
    const std::string FRAG_SHADER = "assets/shaders/bin/skinning.frag.spv";
    const uint32_t Mvp_BINDING = 0;
    const uint32_t InverseBindMatrix_BINDING = 1;
    const uint32_t InverseBinding_Mvp_SET = 0;


    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(SkinVertex) * vertices.size();

        BP->createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertexBuffer, vertexBufferMemory);

        void *data;
        vkMapMemory(BP->device, vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(BP->device, vertexBufferMemory);
        std::cout << "[Skin] Vertex buffer created\n";
    }

    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        BP->createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         indexBuffer, indexBufferMemory);

        void *data;
        vkMapMemory(BP->device, indexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(BP->device, indexBufferMemory);

        std::cout << "[Skin] Index buffer created\n";
    }
};