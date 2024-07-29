#pragma once

#include "render-system/render-system.hpp"
#include "common.hpp"

struct MetallicUniformBufferObject {
    alignas(16) glm::mat4 model;
};
struct MetallicRenderSystemData {
    glm::mat4 model;
};

struct MetallicSystemVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 inColor = glm::vec4(1.0f);
    glm::vec4 tangent;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(MetallicSystemVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(MetallicSystemVertex,
                                                                                      pos)),     sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(MetallicSystemVertex,
                                                                                      normal)),  sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(MetallicSystemVertex,
                                                                                      uv)),      sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(MetallicSystemVertex,
                                                                                      inColor)), sizeof(glm::vec4), COLOR},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(MetallicSystemVertex,
                                                                                      tangent)), sizeof(glm::vec4), TANGENT}

        };
    }
};

class MetallicRenderSystem : public RenderSystem<MetallicRenderSystemData, MetallicSystemVertex> {
public:
    MetallicRenderSystem(std::string pId) : RenderSystem(pId) {}

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes = {};
        auto basePoolSizes = getBasePoolSizes();
        poolSizes.uniformBlocksInPool = 1 + basePoolSizes.uniformBlocksInPool;
        poolSizes.texturesInPool = 3 + basePoolSizes.texturesInPool;// 1 for base texture
        poolSizes.setsInPool = 1 + basePoolSizes.setsInPool; // 1 for model
        return poolSizes;
    }


    void pipelinesAndDescriptorSetsInit() override {
        P.create();
        DS.init(BP, &DSL, {&BaseTexture, &MetallicTexture, &NormalTexture});
        GDS.init(BP, &GDSL, {});
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        P.cleanup();
        DS.cleanup();
        GDS.cleanup();
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {

        P.bind(commandBuffer);
        DS.bind(commandBuffer, P, SET_ID, currentImage);

        GDS.bind(commandBuffer, P, GLOBAL_SET_ID, currentImage);

        AmbientLightUniformBuffer ambientUbo{};
//        ambientUbo.cxp = glm::vec3(1.0f, 0.5f, 0.5f) * 0.2f;
//        ambientUbo.cxn = glm::vec3(0.9f, 0.6f, 0.4f) * 0.2f;
//        ambientUbo.cyp = glm::vec3(0.3f, 1.0f, 1.0f) * 0.2f;
//        ambientUbo.cyn = glm::vec3(0.5f, 0.5f, 0.5f) * 0.2f;
//        ambientUbo.czp = glm::vec3(0.8f, 0.2f, 0.4f) * 0.2f;
//        ambientUbo.czn = glm::vec3(0.3f, 0.6f, 0.7f) * 0.2f;
        updateAmbient(currentImage);
        bindVertexBuffers(commandBuffer);

    }

    void updateUniformBuffers(uint32_t currentImage, MetallicRenderSystemData data) override {
        MetallicUniformBufferObject ubo{};
        ubo.model = data.model;

        DS.map((int) currentImage, &ubo, MODEL_DATA_BINDING);

        updateGlobalBuffers(currentImage);
    }

protected:
    std::string VERT_SHADER = "assets/shaders/bin/metallic.vert.spv";
    std::string FRAG_SHADER = "assets/shaders/bin/metallic.frag.spv";

    void localCleanup() override {
        P.destroy();
        DSL.cleanup();
        BaseTexture.cleanup();
        NormalTexture.cleanup();
        MetallicTexture.cleanup();
    }

    void localInit() override {
        VD.init(BP, MetallicSystemVertex::getBindingDescription(), MetallicSystemVertex::getDescriptorElements());
        GDSL.init(BP, getGDSLBindings());

        DSL.init(BP, {
                {MODEL_DATA_BINDING,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                sizeof(MetallicUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                   1},
                {METALLIC_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1,                                   1},
                {NORMAL_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2,                                   1}
        });

        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&GDSL, &DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

        initTextures();

    }


private:
    VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
    Pipeline P;
    DescriptorSetLayout DSL;
    DescriptorSet DS;


    int SET_ID = 1;
    int GLOBAL_SET_ID = 0;
    uint32_t MODEL_DATA_BINDING = 0;
    uint32_t BASE_TEXTURE_BINDING = 1;
    uint32_t METALLIC_TEXTURE_BINDING = 2;
    uint32_t NORMAL_TEXTURE_BINDING = 3;

    Texture BaseTexture;
    Texture MetallicTexture;
    Texture NormalTexture;

    void initTextures() {
        // Check if the key exists
        if (texturesInfo.find("base") != texturesInfo.end()) {
            TextureInfo base = texturesInfo["base"];
            BaseTexture.init(BP, base.path, base.format, base.initSampler);
        } else {
            throw std::runtime_error("PepsimanRenderSystem: Texture with key 'base' not found.");
        }

        if (texturesInfo.find("metallic") != texturesInfo.end()) {
            TextureInfo metallic = texturesInfo["metallic"];
            MetallicTexture.init(BP, metallic.path, metallic.format, metallic.initSampler);
        } else {
            throw std::runtime_error("PepsimanRenderSystem: Texture with key 'metallic' not found.");
        }

        if (texturesInfo.find("normal") != texturesInfo.end()) {
            TextureInfo normal = texturesInfo["normal"];
            NormalTexture.init(BP, normal.path, normal.format, normal.initSampler);
        } else {
            throw std::runtime_error("PepsimanRenderSystem: Texture with key 'normal' not found.");
        }

    }


};