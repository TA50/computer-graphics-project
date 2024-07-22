#pragma once

#include "render-system/render-system.hpp"
#include "common.hpp"

#define AnimatedSkin_MAX_JOINTS_COUNT 100

struct AnimatedSkinUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 jointTransformMatrices[AnimatedSkin_MAX_JOINTS_COUNT];
};

struct AnimatedSkinRenderSystemData {
    glm::mat4 model;
    glm::mat4 jointTransformMatrices[AnimatedSkin_MAX_JOINTS_COUNT];
};

struct AnimatedSkinSystemVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;
    glm::vec3 inColor;
    glm::vec4 inTan;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(AnimatedSkinSystemVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }


    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      pos)),          sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      normal)),       sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      uv)),           sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SINT,   static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      jointIndices)), sizeof(glm::vec4), OTHER},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      jointWeights)), sizeof(glm::vec4), OTHER},

                {0, 5, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      inColor)),      sizeof(glm::vec3), COLOR},
                {0, 6, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(AnimatedSkinSystemVertex,
                                                                                      inTan)),        sizeof(glm::vec4), TANGENT},

        };
    }
};

class AnimatedSkinRenderSystem : public RenderSystem<AnimatedSkinRenderSystemData, AnimatedSkinSystemVertex> {
public:
    AnimatedSkinRenderSystem(std::string pId) : RenderSystem(pId) {}

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes = {};
        auto basePoolSizes = getBasePoolSizes();
        poolSizes.uniformBlocksInPool = 1 + basePoolSizes.uniformBlocksInPool;
        poolSizes.texturesInPool = 1 + basePoolSizes.texturesInPool;; // 1 for base texture
        poolSizes.setsInPool = 1 + basePoolSizes.setsInPool;; // 1 for model, 1 for camera and light
        return poolSizes;
    }

    void pipelinesAndDescriptorSetsInit() override {
        P.create();
        DS.init(BP, &DSL, {&BaseTexture, &NormalTexture});
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


        bindVertexBuffers(commandBuffer);

    }

    void updateUniformBuffers(uint32_t currentImage, AnimatedSkinRenderSystemData data) override {
        AnimatedSkinUniformBufferObject ubo{};
        ubo.model = data.model;
        for (int i = 0; i < AnimatedSkin_MAX_JOINTS_COUNT; i++) {
            ubo.jointTransformMatrices[i] = data.jointTransformMatrices[i];
        }

        DS.map((int) currentImage, &ubo, MODEL_DATA_BINDING);
        updateGlobalBuffers(currentImage);
    }

protected:
    std::string VERT_SHADER = "assets/shaders/bin/animated-skin.vert.spv";
    std::string FRAG_SHADER = "assets/shaders/bin/animated-skin.frag.spv";

    void localCleanup() override {
        P.destroy();
        DSL.cleanup();
        BaseTexture.cleanup();
        NormalTexture.cleanup();
    }

    void localInit() override {
        VD.init(BP, AnimatedSkinSystemVertex::getBindingDescription(),
                AnimatedSkinSystemVertex::getDescriptorElements());
        GDSL.init(BP, getGDSLBindings());

        DSL.init(BP, {
                {MODEL_DATA_BINDING,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                sizeof(AnimatedSkinUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                       1},
                {NORMAL_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1,                                       1}
        });

        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&GDSL, &DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

        initTextures();
        std::cout << "AnimatedSkinRenderSystem initialized" << std::endl;
    }


private:
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
    Pipeline P;
    DescriptorSetLayout DSL;
    DescriptorSet DS;


    int SET_ID = 1;
    int GLOBAL_SET_ID = 0;
    uint32_t MODEL_DATA_BINDING = 0;
    uint32_t BASE_TEXTURE_BINDING = 1;
    uint32_t NORMAL_TEXTURE_BINDING = 2;

    Texture BaseTexture;
    Texture NormalTexture;

    void initTextures() {
        // Check if the key exists
        if (texturesInfo.find("base") != texturesInfo.end()) {
            TextureInfo base = texturesInfo["base"];
            BaseTexture.init(BP, base.path, base.format, base.initSampler);
        } else {
            throw std::runtime_error("AnimatedSkinRenderSystem: Texture with key 'base' not found.");
        }
        if (texturesInfo.find("normal") != texturesInfo.end()) {
            TextureInfo base = texturesInfo["normal"];
            NormalTexture.init(BP, base.path, base.format, base.initSampler);
        } else {
            throw std::runtime_error("AnimatedSkinRenderSystem: Texture with key 'normal' not found.");
        }

    }


};