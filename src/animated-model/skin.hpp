#pragma once

#include <string>
#include <vector>
#include "animated-model/joint.hpp"
#include "modules/Starter.hpp"
#include "animated-model/joint.hpp"
#include "camera.hpp"
#include "printer.hpp"
#include "helper-structs.hpp"
#include "animated-model/animation.hpp"

#define MAX_JOINTS_COUNT 100

struct SkinVertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;
    glm::vec3 inColor;

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
                {0, 3, VK_FORMAT_R32G32B32A32_SINT,   static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      jointIndices)), sizeof(glm::vec4), OTHER},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      jointWeights)), sizeof(glm::vec4), OTHER},

                {0, 5, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(SkinVertex,
                                                                                      inColor)),      sizeof(glm::vec3), COLOR}

        };
    }
};

struct SkinMvpObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 jointTransformMatrices[MAX_JOINTS_COUNT];
    glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
};

struct SkinInverseBindMatrixObject {
    alignas(16) glm::mat4 inverseBindMatrices[MAX_JOINTS_COUNT];
};

class Skin {
public:
    std::vector<Animation> animations;

    explicit Skin(std::string name, int jointsCount) : name(std::move(name)), jointsCount(jointsCount) {
        rootJointIndex = -1;

        joints = std::unordered_map<int, Joint *>();
        inverseBindMatrices = std::unordered_map<int, glm::mat4>();
        jointMatrices = std::unordered_map<int, glm::mat4>();

        vertices = std::vector<SkinVertex>();
        indices = std::vector<uint32_t>();

        BP = nullptr;


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
        VD.init(bp, SkinVertex::getBindingDescription(), SkinVertex::getDescriptorElements());
        DSL.init(bp, {
                {Mvp_BINDING,               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SkinMvpObject),               1},
                {InverseBindMatrix_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SkinInverseBindMatrixObject), 1},
                {Base_Texture_Binding,      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                   1},
        });
        BaseTexture.init(bp, baseTexture);

        P.init(bp, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              VK_CULL_MODE_BACK_BIT, false);
        createVertexBuffer();
        createIndexBuffer();

        updateJointMatrices();

    }

    void createPipelineAndDescriptorSets() {
        P.create();
        DS.init(BP, &DSL, {&BaseTexture});
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
    }

    void render(uint32_t currentImage, AxisInput input) {

        updateJointMatrices();
        updateUniformBuffers(currentImage);
    }

    void move(glm::vec3 translation) {
        this->translation += translation;
    }

    void setWorldMatrix(glm::mat4 mat) {
        worldMatrix = mat;
    }

    void updateWorldMatrix() {

        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rot = glm::rotate(rot, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rot = glm::rotate(rot, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        worldMatrix = glm::translate(glm::mat4(1.0f), translation) *
                      rot *
                      glm::scale(glm::mat4(1.0f), scaling);

        translation = glm::vec3(0);
        rotation = glm::vec3(0);
        scaling = glm::vec3(1);
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
        jointMatrices[jointIndex] = joint->getTransformedMatrix();
        joints[jointIndex] = joint;
    }

    void addVertex(SkinVertex vertex) {
        vertices.push_back(vertex);
    }

    void addIndex(uint32_t index) {
        indices.push_back(index);
    }


    void addAnimation(const Animation &animation) {
        animations.push_back(animation);
    }


    // Setters

    void setTranslation(glm::vec3 translation) {
        this->translation = translation;
    }

    void setScaling(glm::vec3 scaling) {
        this->scaling = scaling;
    }

    void setRotation(glm::vec3 rotation) {
        this->rotation = rotation;
    }

    void setRootJoint(Joint *joint, int index) {
        rootJointIndex = index;
        rootJoint = joint;
    }

    void setActiveAnimation(uint32_t index) {
        activeAnimation = index;
    }

    // Getters
    Joint *getJoint(int index) {
        return joints[index];
    }

    glm::vec3 getPosition() {
    auto Wm = getModelMatrix();
    return Wm[3];
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

    Joint *getRootJoint() const {
        return rootJoint;
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

        auto M = getTransformedWorldMatrix();
        return rootJoint->getGlobalMatrix() * M;
    }

    glm::mat4 getTransformedWorldMatrix(){
        glm::mat4 M = worldMatrix;
        M = glm::scale(M, scaling);
        M = glm::rotate(M, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        M = glm::translate(M, translation);
        return M;
    }


    void updateAnimation(float deltaTime) {

        if (activeAnimation > static_cast<uint32_t>(animations.size()) - 1) {
            std::cout << "No animation with index " << activeAnimation << std::endl;
            return;
        }
        auto animation = &animations[activeAnimation];
        animation->currentTime += deltaTime;
        if (animation->currentTime > animation->end) {
            animation->currentTime -= animation->end;
        }

        for (auto &channel: animation->channels) {
            auto sampler = animation->samplers[channel.samplerIndex];
            for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
                if (sampler.interpolation != "LINEAR") {
                    std::cout << "This sample only supports linear interpolations\n";
                    continue;
                }

                // Get the input keyframe values for the current time stamp
                if ((animation->currentTime >= sampler.inputs[i]) &&
                    (animation->currentTime <= sampler.inputs[i + 1])) {
                    float a = (animation->currentTime - sampler.inputs[i]) /
                              (sampler.inputs[i + 1] - sampler.inputs[i]);
                    if (channel.path == "translation") {
                        channel.joint->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
                    }

                    if (channel.path == "rotation") {
                        glm::quat q1;
                        q1.x = sampler.outputsVec4[i].x;
                        q1.y = sampler.outputsVec4[i].y;
                        q1.z = sampler.outputsVec4[i].z;
                        q1.w = sampler.outputsVec4[i].w;

                        glm::quat q2;
                        q2.x = sampler.outputsVec4[i + 1].x;
                        q2.y = sampler.outputsVec4[i + 1].y;
                        q2.z = sampler.outputsVec4[i + 1].z;
                        q2.w = sampler.outputsVec4[i + 1].w;

                        channel.joint->rotation = glm::normalize(glm::slerp(q1, q2, a));
                    }
                    if (channel.path == "scale") {
                        channel.joint->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
                    }
                }
            }
        }
    }

    void updateJointMatrices() {

        auto rootMat = getJointMatrix(rootJoint);

        glm::mat4 inverseTransform = glm::inverse(rootMat);
        for (auto &[index, joint]: joints) {
            // Apply joint transformation and inverse bind matrix
            jointMatrices[index] = getJointMatrix(joint) * inverseBindMatrices[index];
            // Transform back to model space
            jointMatrices[index] = inverseTransform * jointMatrices[index];

        }


    }

    static glm::mat4 getJointMatrix(Joint *joint) {
        glm::mat4 jointMatrix = joint->getTransformedMatrix();
        Joint *currentParent = joint->parent;
        while (currentParent) {
            jointMatrix = currentParent->getTransformedMatrix() * jointMatrix;
            currentParent = currentParent->parent;
        }
        return jointMatrix;
    }


protected:

    glm::mat4 worldMatrix = glm::mat4(1.0f);
    glm::vec3 translation = glm::vec3(0.0f);
    glm::vec3 scaling = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(0.0f);

    uint32_t activeAnimation = 0;
    int rootJointIndex;
    Joint *rootJoint;
    int jointsCount;
    std::string name;
    std::unordered_map<int, Joint *> joints;
    std::vector<SkinVertex> vertices;
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

    const std::string VERT_SHADER = "assets/shaders/bin/skinning.vert.spv";
    const std::string FRAG_SHADER = "assets/shaders/bin/skinning.frag.spv";
    const uint32_t InverseBinding_Mvp_SET = 0;
    const uint32_t Mvp_BINDING = 0;
    const uint32_t InverseBindMatrix_BINDING = 1;
    const uint32_t Base_Texture_Binding = 2;


    void mapInverseBindMatrices(uint32_t currentImage) {
        SkinInverseBindMatrixObject inverseBindMatrixObject{};
        for (auto kv: inverseBindMatrices) {
            inverseBindMatrixObject.inverseBindMatrices[kv.first] = kv.second;
        }

        DS.map(currentImage, &inverseBindMatrixObject, InverseBindMatrix_BINDING);
    }


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


    }


    void updateUniformBuffers(uint32_t currentImage) {

        SkinMvpObject mvpObject{};
        mvpObject.model = getModelMatrix();
        mvpObject.view = camera->matrices.view;
        mvpObject.projection = camera->matrices.perspective;

        for (auto &[index, matrix]: jointMatrices) {
            mvpObject.jointTransformMatrices[index] = matrix;
        }


        DS.map(currentImage, &mvpObject, Mvp_BINDING);
    }


};