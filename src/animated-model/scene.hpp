#pragma once

#include <string>
#include <vector>
#include "animated-model/joint.hpp"
#include "modules/Starter.hpp"
#include "animated-model/joint.hpp"
#include "camera.hpp"

#define MAX_JOINTS_COUNT 100

struct SceneVertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(SceneVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(SceneVertex,
                                                                                      pos)),          sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(SceneVertex,
                                                                                      normal)),       sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(SceneVertex,
                                                                                      uv)),           sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SINT,   static_cast<uint32_t >(offsetof(SceneVertex,
                                                                                      jointIndices)), sizeof(glm::vec4), OTHER},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(SceneVertex,
                                                                                      jointWeights)), sizeof(glm::vec4), OTHER}
        };
    }
};

struct SceneMvpObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 jointTransformMatrices[MAX_JOINTS_COUNT];
};

struct SceneInverseBindMatrixObject {
    alignas(16) glm::mat4 inverseBindMatrices[MAX_JOINTS_COUNT];
};

class Scene {
public:
    explicit Scene(std::string name, int jointsCount) : name(std::move(name)), jointsCount(jointsCount) {
        rootJointIndex = -1;

        joints = std::unordered_map<int, Joint *>();
        inverseBindMatrices = std::unordered_map<int, glm::mat4>();
        jointMatrices = std::unordered_map<int, glm::mat4>();

        vertices = std::vector<SceneVertex>();
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
        DPSZs.texturesInPool = 1;
        DPSZs.setsInPool = 2;
        return DPSZs;
    }


    void init(BaseProject *bp, Camera *_camera, std::string baseTexture) {
        BP = bp;
        camera = _camera;
        VD.init(bp, SceneVertex::getBindingDescription(), SceneVertex::getDescriptorElements());
        DSL.init(bp, {
                {Mvp_BINDING,               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SceneMvpObject),               1},
                {InverseBindMatrix_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SceneInverseBindMatrixObject), 1},
                {Base_Texture_Binding,      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                   1},
        });
        BaseTexture.init(bp, baseTexture);

        P.init(bp, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              VK_CULL_MODE_BACK_BIT, false);
        createVertexBuffer();
        createIndexBuffer();


//        updateJointMatrices(joints[rootJointIndex]);
//        updateJointMatrices();

    }

    void createPipelineAndDescriptorSets() {
        P.create();
        std::cout << "[Scene]: Pipeline created\n";
        DS.init(BP, &DSL, {&BaseTexture});
        std::cout << "[Scene]: Descriptor Set Created\n";
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

        mapInverseBindMatrices(currentImage);
        modelMatrix = getModelMatrix();
    }

    void updateUniformBuffers(uint32_t currentImage) {

        updateJointMatrices();

        std::cout << "Updating uniform buffers\n";
        SceneMvpObject mvpObject{};
        mvpObject.model = joints[rootJointIndex]->getLocalMatrix();
        mvpObject.view = camera->matrices.view;
        mvpObject.projection = camera->matrices.perspective;

        for (auto &[index, matrix]: jointMatrices) {
            mvpObject.jointTransformMatrices[index] = matrix;
        }

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
        BaseTexture.cleanup();

        vkDestroyBuffer(BP->device, vertexBuffer, nullptr);
        vkFreeMemory(BP->device, vertexBufferMemory, nullptr);

        vkDestroyBuffer(BP->device, indexBuffer, nullptr);
        vkFreeMemory(BP->device, indexBufferMemory, nullptr);
    }

    void addJoint(Joint *joint, glm::mat4 inverseBindMatrix, int jointIndex) {
        joints[jointIndex] = joint;
        inverseBindMatrices[jointIndex] = inverseBindMatrix;
        joint->updateTransformedMatrix();
        jointMatrices[jointIndex] = joint->getTransformedMatrix();
        joints[jointIndex] = joint;
    }

    void addVertex(SceneVertex vertex) {
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

    std::vector<SceneVertex> getVertices() {
        std::vector<int> normalIndices;
        return vertices;
    }


    glm::mat4 getModelMatrix() {
        return joints[rootJointIndex]->getGlobalMatrix();
    }

    // POI: Update the joint matrices from the current animation frame and pass them to the GPU
//    void updateJointMatrices(Joint *joint) {
//        glm::mat4 inverseTransform = glm::inverse(Joint::getJointMatrix(joint));
//        for (auto kv: joints) {
//            jointMatrices[kv.first] = Joint::getJointMatrix(kv.second) * inverseBindMatrices[kv.first];
//            jointMatrices[kv.first] = inverseTransform * jointMatrices[kv.first];
//        }
//        for (auto &child: joint->children) {
//            updateJointMatrices(child);
//        }
//    }
    void updateJointMatrices() {
        // Update transformed matrices for each joint
        auto modelMatrix = getJointMatrix(joints[rootJointIndex]);

        glm::mat4 inverseTransform = glm::inverse(modelMatrix);
        for (auto &[index, joint]: joints) {
            // Apply joint transformation and inverse bind matrix
            glm::mat4 globalMatrix = joint->getGlobalMatrix();
            jointMatrices[index] = globalMatrix * inverseBindMatrices[index];
            // Transform back to model space
            jointMatrices[index] = inverseTransform * jointMatrices[index];

        }
    }

    glm::mat4 getJointMatrix(Joint * joint){
        glm::mat4              jointMatrix    = joint->getTransformedMatrix();
        Joint * currentParent = joint->parent;
        while (currentParent)
        {
            jointMatrix    = currentParent->getTransformedMatrix() * jointMatrix;
            currentParent = currentParent->parent;
        }
        return jointMatrix;
    }

protected:
    glm::mat4 modelMatrix;
    int rootJointIndex;
    int jointsCount;
    std::string name;
    std::unordered_map<int, Joint *> joints;
    std::vector<SceneVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<int, glm::mat4> inverseBindMatrices;
    std::unordered_map<int, glm::mat4> jointMatrices{};


    DescriptorSetLayout DSL;
    DescriptorSet DS;

    Texture BaseTexture;

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

    const std::string VERT_SHADER = "assets/shaders/bin/Scenening.vert.spv";
    const std::string FRAG_SHADER = "assets/shaders/bin/Scenening.frag.spv";
    const uint32_t InverseBinding_Mvp_SET = 0;
    const uint32_t Mvp_BINDING = 0;
    const uint32_t InverseBindMatrix_BINDING = 1;
    const uint32_t Base_Texture_Binding = 2;


    void mapInverseBindMatrices(uint32_t currentImage) {
        SceneInverseBindMatrixObject inverseBindMatrixObject{};
        for (auto kv: inverseBindMatrices) {
            inverseBindMatrixObject.inverseBindMatrices[kv.first] = kv.second;
        }

        DS.map(currentImage, &inverseBindMatrixObject, InverseBindMatrix_BINDING);
    }

//    SceneMvpObject getMvpObject(){
//        mvpObject
//    }

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(SceneVertex) * vertices.size();

        BP->createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         vertexBuffer, vertexBufferMemory);

        void *data;
        vkMapMemory(BP->device, vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(BP->device, vertexBufferMemory);
        std::cout << "[Scene] Vertex buffer created\n";
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

        std::cout << "[Scene] Index buffer created\n";
    }
};