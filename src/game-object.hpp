// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


struct GameObjectVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 inColor = glm::vec4(1.0f);

    void setColor(glm::vec4 color) {
        inColor = color;
    }

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
                                                                                      inColor)), sizeof(glm::vec4), COLOR}
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

    void setWorldMatrix(glm::mat4 W) {
        Wm = W;
    }
    void setCullMode(VkCullModeFlagBits flag) {
        this->cullMode = flag;
    }

    void setVertices(std::vector<GameObjectVertex> v) {
        vertices = v;
    }

    void setIndices(std::vector<uint32_t> i) {
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
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

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

        auto model = getModel();

        GameObjectUniformBufferObject ubo = GameObjectUniformBufferObject(model,
                                                                          camera->matrices.view,
                                                                          camera->matrices.perspective);


        DS.map((int) currentImage, &ubo, (int) SET_ID);
    }


    void move(glm::vec3 pos) {
        translation = pos;
    }

    void scale(glm::vec3 scale) {
        scaling = scale;
    }

    void rotate(glm::vec3 degrees) {
        rotation = degrees;
    }

    void updateWorld() {
        // scale
        Wm = glm::scale(Wm, scaling);
        Wm = glm::rotate(Wm, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        Wm = glm::translate(Wm, translation);

        rotation = glm::vec3(0.0);
        translation = glm::vec3(0.0);
        scaling = glm::vec3(1.0);
    }

private:
    std::string name;
    Camera *camera{};
    BaseProject *BP{};


    glm::vec3 translation = glm::vec3(0.0);
    glm::vec3 scaling = glm::vec3(1.0);
    glm::vec3 rotation = glm::vec3(0.0);

    Pipeline P;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;

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
    glm::mat4 Wm = glm::mat4(1.0f);
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


    glm::mat4 getModel() {
        // scale
        glm::mat4 model = glm::scale(Wm, scaling);
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        model = glm::translate(model, translation);
        return model;
    }

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
