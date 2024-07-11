// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "common.hpp"

struct GameObjectVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 inColor;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(GameObjectVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      pos)),     sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      normal)),  sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      uv)),      sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      inColor)), sizeof(glm::vec3), COLOR}
        };
    }
};

struct GameObjectUniformBufferObject : BaseUniformBufferObject {
    GameObjectUniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
            : BaseUniformBufferObject(model, view, projection) {}
};


class GameObject {
public:
    void setName(std::string n) {
        name = n;
    }

    void setVertices(std::vector<GameObjectVertex> &v) {
        vertices = v;
    }

    void setIndices(std::vector<uint32_t> &i) {
        indices = i;
    }

    void setModel(std::string filePath, ModelType type) {
        modelPath = filePath;
        modelType = type;
        loaded = true;
    }

    void setBaseTexture(std::string filePath, VkFormat Fmt, bool initSampler = false) {
        baseTexturePath = filePath;
        baseTextureFormat = Fmt;
        baseTextureInitSampler = initSampler;

    }

    PoolSizes getPoolSizes() {
        PoolSizes poolSizes{};
        poolSizes.setsInPool = 1;
        poolSizes.uniformBlocksInPool = 1;
        poolSizes.texturesInPool = 1;
        return poolSizes;
    }

    void init(BaseProject *bp, Camera *pCamera) {
        this->camera = pCamera;
        this->BP = bp;
        DSL.init(bp, {
                {MVP_BINDING,          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                sizeof(GameObjectUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
        });
        VD.init(bp, GameObjectVertex::getBindingDescription(),
                GameObjectVertex::getDescriptorElements());

        BaseTexture.init(BP, baseTexturePath, baseTextureFormat, baseTextureInitSampler);


        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});

        if (loaded) {
            M.init(bp, &VD, modelPath, modelType);
        } else {
            createVertexBuffer();
            createIndexBuffer();
        }
    }


    void pipelinesAndDescriptorSetsInit() {
        P.create();
        DS.init(BP, &DSL, {&BaseTexture});
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
        if (loaded) {
            M.cleanup();
        } else {
            vkDestroyBuffer(BP->device, vertexBuffer, nullptr);
            vkFreeMemory(BP->device, vertexBufferMemory, nullptr);

            vkDestroyBuffer(BP->device, indexBuffer, nullptr);
            vkFreeMemory(BP->device, indexBufferMemory, nullptr);
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        P.bind(commandBuffer);
        DS.bind(commandBuffer, P, SET_ID, currentImage);
        if (loaded) {
            M.bind(commandBuffer);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(M.indices.size()), 1, 0, 0, 0);

        } else {
            VkBuffer vertexBuffers[] = {vertexBuffer};
            // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0,
                                 VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }

    }

    void render(uint32_t currentImage) {
        GameObjectUniformBufferObject ubo = GameObjectUniformBufferObject(Wm, camera->matrices.view,
                                                                          camera->matrices.perspective);
        DS.map((int) currentImage, &ubo, (int) SET_ID);
    }


    void move(glm::vec3 pos) {
        Wm = glm::translate(Wm, pos);
    }

    void scale(glm::vec3 scale) {
        Wm = glm::scale(Wm, scale);
    }

    void rotate(float angle, glm::vec3 axis) {
        Wm = glm::rotate(Wm, angle, axis);
    }

private:
    std::string name;
    Camera *camera{};
    BaseProject *BP{};

    Pipeline P;
    DescriptorSet DS;
    DescriptorSetLayout DSL;
    VertexDescriptor VD;

    Texture BaseTexture{};

    VkFormat baseTextureFormat;
    bool baseTextureInitSampler;
    std::string baseTexturePath;

    Model M;
    std::string modelPath;
    ModelType modelType;

    std::vector<GameObjectVertex> vertices;
    glm::mat4 Wm{};
    std::vector<unsigned char> loadedModelVertices{};
    bool loaded = false;
    std::vector<uint32_t> indices;


    VkBuffer vertexBuffer{};
    VkDeviceMemory vertexBufferMemory{};


    VkBuffer indexBuffer{};
    VkDeviceMemory indexBufferMemory{};

    uint32_t MVP_BINDING = 0;
    uint32_t BASE_TEXTURE_BINDING = 1;

    uint32_t SET_ID = 0;

    const std::string VERT_SHADER = "assets/shaders/bin/game-object.vert.spv";
    const std::string FRAG_SHADER = "assets/shaders/bin/game-object.frag.spv";

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(GameObjectVertex) * vertices.size();

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

};
