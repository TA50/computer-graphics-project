#pragma once

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "light-object.hpp"
#include "common.hpp"


struct CameraUniformBuffer {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::vec3 position;
};

struct LightUniformBuffer {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec4 color;
    alignas(16) float specularGamma = 256.0f;
};

struct AmbientLightUniformBuffer {
    alignas(16) glm::vec3 cxp = glm::vec3(1);
    alignas(16) glm::vec3 cxn = glm::vec3(1);
    alignas(16) glm::vec3 cyp = glm::vec3(1);
    alignas(16) glm::vec3 cyn = glm::vec3(1);
    alignas(16) glm::vec3 czp = glm::vec3(1);
    alignas(16) glm::vec3 czn = glm::vec3(1);
};

template<typename TRenderSystemData, typename TVertex>
class RenderSystem {

public:
    RenderSystem(std::string pId) : id(pId) {
    }

    PoolSizes getBasePoolSizes(){
        PoolSizes poolSizes;
        poolSizes.uniformBlocksInPool = 3;
        poolSizes.texturesInPool = 0;
        poolSizes.setsInPool = 1;

        return poolSizes;
    }
    void addVertices(std::vector<TVertex> verts, std::vector<uint32_t> inds) {
        vertices = verts;
        indices = inds;
    }

    void init(BaseProject *bp, Camera *pCamera, Light *plight) {
        camera = pCamera;
        light = plight;
        BP = bp;


        localInit();

        createVertexBuffer();
        createIndexBuffer();
    }

    virtual PoolSizes getPoolSizes() = 0;


    virtual void pipelinesAndDescriptorSetsInit() = 0;

    virtual void pipelinesAndDescriptorSetsCleanup() = 0;

    virtual void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) = 0;


    virtual void updateUniformBuffers(uint32_t currentImage, TRenderSystemData ubo) = 0;

    std::string getId() {
        return id;
    }

    void cleanup() {
        vkDestroyBuffer(BP->device, vertexBuffer, nullptr);
        vkFreeMemory(BP->device, vertexBufferMemory, nullptr);

        vkDestroyBuffer(BP->device, indexBuffer, nullptr);
        vkFreeMemory(BP->device, indexBufferMemory, nullptr);

        VD.cleanup();
        GDSL.cleanup();
    }

    void setTextures(std::unordered_map<std::string, TextureInfo> texsInfo) {
        this->texturesInfo = texsInfo;
    }

protected:

    std::vector<DescriptorSetLayoutBinding> getGDSLBindings() {
        return {
                {CAMERA_DATA_BINDING,  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS,
                        sizeof(CameraUniformBuffer),       1},
                {LIGHT_DATA_BINDING,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS,
                        sizeof(LightUniformBuffer),        1},

                {AMBIENT_DATA_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS,
                        sizeof(AmbientLightUniformBuffer), 1},

        };
    }

    std::string VERT_SHADER;
    std::string FRAG_SHADER;
    std::vector<TVertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer{};
    VkDeviceMemory vertexBufferMemory{};
    VertexDescriptor VD;

    DescriptorSetLayout GDSL;
    DescriptorSet GDS;
    const uint32_t AMBIENT_DATA_BINDING = 2;
    const uint32_t LIGHT_DATA_BINDING = 1;
    const uint32_t CAMERA_DATA_BINDING = 0;


    VkBuffer indexBuffer{};
    VkDeviceMemory indexBufferMemory{};

    std::string id;
    Camera *camera;
    BaseProject *BP;
    Light *light;

    std::unordered_map<std::string, TextureInfo> texturesInfo;

    virtual void localInit() = 0;

    virtual void localCleanup() = 0;

    void updateAmbient(int currentImage){
        AmbientLightUniformBuffer ubo{};
        ubo.cxn = light->ambientColors.cxn;
        ubo.cxp = light->ambientColors.cxp;
        ubo.cyn = light->ambientColors.cyn;
        ubo.cyp = light->ambientColors.cyp;
        ubo.czn = light->ambientColors.czn;
        ubo.czp = light->ambientColors.czp;

        GDS.map(currentImage, &ubo, AMBIENT_DATA_BINDING);
    }
    void bindVertexBuffers(VkCommandBuffer commandBuffer) {
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

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

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
