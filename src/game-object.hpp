// This has been adapted from the Vulkan tutorial
#pragma once

#include <light-object.hpp>

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
    GameObject(std::string _id) : id(_id) {

    }

    void setLight(Light *lightObject) {
        lubo = lightObject->getUBO();
        GDSL = lightObject->getDSL();
        GDS = lightObject->getDS();
        this->lightObject = lightObject;
    }


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

    void setMetalicTexture(std::string filePath, VkFormat Fmt, bool initSampler = false) {
        metalicTexturePath = filePath;
        metalicTextureFormat = Fmt;
        metalicTextureInitSampler = initSampler;
    }

    void setRoughnessTexture(std::string filePath, VkFormat Fmt, bool initSampler = false) {
        roughnessTexturePath = filePath;
        roughnessTextureFormat = Fmt;
        roughnessTextureInitSampler = initSampler;
    }

    void setDiffuseTexture(std::string filePath, VkFormat Fmt, bool initSampler = false) {
        diffuseTexturePath = filePath;
        diffuseTextureFormat = Fmt;
        diffuseTextureInitSampler = initSampler;
    }

    std::string getId() {
        return id;
    }

    PoolSizes getPoolSizes() {
        PoolSizes poolSizes{};
        poolSizes.setsInPool = 1;
        poolSizes.uniformBlocksInPool = 1;
        poolSizes.texturesInPool = 4;
        return poolSizes;
    }

    void init(BaseProject *bp, Camera *pCamera) {
        this->camera = pCamera;
        this->BP = bp;
        DSL.init(bp, {
                {MVP_BINDING,               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                     sizeof(GameObjectUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING,      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
                {METALIC_TEXTURE_BINDING,   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
                {ROUGHNESS_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
                {DIFFUSE_TEXTURE_BINDING,   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
        });
        VD.init(bp, GameObjectVertex::getBindingDescription(),
                GameObjectVertex::getDescriptorElements());

        BaseTexture.init(BP, baseTexturePath, baseTextureFormat, baseTextureInitSampler);
        if (!metalicTexturePath.empty()) {
            MetalicTexture.init(BP, metalicTexturePath, metalicTextureFormat, metalicTextureInitSampler);
        }
        if (!diffuseTexturePath.empty()) {
            DiffuseTexture.init(BP, diffuseTexturePath, diffuseTextureFormat, diffuseTextureInitSampler);
        }
        if (!roughnessTexturePath.empty()) {
            RoughnessTexture.init(BP, roughnessTexturePath, roughnessTextureFormat, roughnessTextureInitSampler);
        }

        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&DSL, &GDSL});
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
        GDS.init(BP, &GDSL, {});
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
        GDS.bind(commandBuffer, P, SET_ID_LIGHT, currentImage);
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
        auto lubo = lightObject->getUBO();
        GDS.map((int) currentImage, &lubo, 0);

    }


    void setTranslation(glm::vec3 pos) {
        translation = pos;
    }

    void move(glm::vec3 pos) {
        translation += pos;
    }

    void scale(glm::vec3 scale) {
        scaling = scale;
    }

    void setRotation(glm::vec3 degrees) {
        rotation = degrees;
    }

    void rotate(glm::vec3 degrees) {
        rotation += degrees;
    }

    void setLocalMatrix(glm::mat4 m) {
        LocalMatrix = m;
    }

    glm::mat4 getLocalMatrix() {
        return LocalMatrix;
    }

    void updateWorld() {
        // scale
        std::cout << name << std::endl;
        std::cout << "Pre" << std::endl;
        Printer::printArrArr(Wm);
        Wm = glm::scale(LocalMatrix, scaling);
        std::cout << "scale x " << scaling.x << " y " << scaling.y << " z " << scaling.z << std::endl;
        Wm = glm::rotate(Wm, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        Wm = glm::translate(Wm, translation);
        std::cout << "After" << std::endl;
        Printer::printArrArr(Wm);

        rotation = glm::vec3(0.0);
        translation = glm::vec3(0.0);
        scaling = glm::vec3(1.0);
    }


    glm::mat4 getModel() {
        // scale
        auto M = LocalMatrix;
//        glm::mat4 model = glm::translate(M, -translation);

        auto model = glm::scale(M, scaling);

        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        model = glm::translate(model, translation);

        return model;
    }

    std::string getName() {
        return name;

    }


    std::vector<glm::vec3> getVertexLocations() {
        std::vector<glm::vec3> vertexLocations{vertices.size()};
        for (const auto &vertex: vertices) {
            vertexLocations.push_back(vertex.pos);
        }
        return vertexLocations;
    }

    glm::vec3 calculateGeometricCenter() {
        std::vector<glm::vec3> vertexLocations = getVertexLocations();
        glm::vec3 center(0.0f);
        for (const auto &vertex: vertexLocations) {
            center += vertex;
        }
        center /= static_cast<float>(vertexLocations.size());
        return center;
    }

    void changeCenterToGeometricCenter() {
        glm::vec3 geometricCenter = calculateGeometricCenter();
        changeModelCenter(geometricCenter);
    }

    void changeModelCenter(const glm::vec3 &newCenter) {
        glm::vec3 currentCenter = calculateGeometricCenter();
        glm::vec3 translationToOrigin = -currentCenter;
        glm::vec3 translationToNewCenter = newCenter;

        glm::mat4 translateToOriginMatrix = glm::translate(glm::mat4(1.0f), translationToOrigin);
        glm::mat4 translateToNewCenterMatrix = glm::translate(glm::mat4(1.0f), translationToNewCenter);

        // Combined transformation matrix: move to origin, then move to new center
        OriginMatrix = translateToNewCenterMatrix;
    }

    VkFormat baseTextureFormat;
    bool baseTextureInitSampler;
    std::string baseTexturePath;

    std::vector<GameObjectVertex> vertices;
    std::vector<uint32_t> indices;
private:
    std::string name;
    std::string id;
    Camera *camera{};
    Light *lightObject;
    BaseProject *BP{};
    glm::mat4 LocalMatrix = glm::mat4(1.0f);
    glm::mat4 OriginMatrix = glm::mat4(1.0f);

    glm::vec3 translation = glm::vec3(0.0);
    glm::vec3 scaling = glm::vec3(1.0);
    glm::vec3 rotation = glm::vec3(0.0);

    Pipeline P;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;

    DescriptorSet DS;
    DescriptorSetLayout DSL;
    VertexDescriptor VD;

    LightUnifromBufferObject lubo{};
    DescriptorSetLayout GDSL;
    DescriptorSet GDS;

    Texture BaseTexture{};
    Texture MetalicTexture{};
    Texture RoughnessTexture{};
    Texture DiffuseTexture{};


    VkFormat metalicTextureFormat;
    bool metalicTextureInitSampler;
    std::string metalicTexturePath;

    VkFormat roughnessTextureFormat;
    bool roughnessTextureInitSampler;
    std::string roughnessTexturePath;

    VkFormat diffuseTextureFormat;
    bool diffuseTextureInitSampler;
    std::string diffuseTexturePath;

    Model M;
    std::string modelPath;
    ModelType modelType;

    glm::mat4 Wm = glm::mat4(1.0f);
    std::vector<unsigned char> loadedModelVertices{};
    bool loaded = false;


    VkBuffer vertexBuffer{};
    VkDeviceMemory vertexBufferMemory{};


    VkBuffer indexBuffer{};
    VkDeviceMemory indexBufferMemory{};

    uint32_t MVP_BINDING = 0;
    uint32_t BASE_TEXTURE_BINDING = 1;
    uint32_t METALIC_TEXTURE_BINDING = 2;
    uint32_t ROUGHNESS_TEXTURE_BINDING = 3;
    uint32_t DIFFUSE_TEXTURE_BINDING = 4;


    uint32_t SET_ID = 0;
    uint32_t SET_ID_LIGHT = 1;

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
