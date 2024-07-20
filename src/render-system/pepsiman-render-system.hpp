#pragma once

#include "render-system/render-system.hpp"
#include "common.hpp"

#define Pepsiman_MAX_JOINTS_COUNT 100

struct PepsimanUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 jointTransformMatrices[Pepsiman_MAX_JOINTS_COUNT];
};

struct PepsimanRenderSystemData {
    glm::mat4 model;
    glm::mat4 jointTransformMatrices[Pepsiman_MAX_JOINTS_COUNT];
};

struct PepsimanSystemVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;
    glm::vec3 inColor;
    glm::vec4 tangent;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(PepsimanSystemVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }


    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      pos)),          sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      normal)),       sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      uv)),           sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SINT,   static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      jointIndices)), sizeof(glm::vec4), OTHER},
                {0, 4, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      jointWeights)), sizeof(glm::vec4), OTHER},

                {0, 5, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      inColor)),      sizeof(glm::vec3), COLOR},
                {0, 6, VK_FORMAT_R32G32B32A32_SFLOAT,    static_cast<uint32_t >(offsetof(PepsimanSystemVertex,
                                                                                      tangent)),      sizeof(glm::vec4), TANGENT},

        };
    }
};

class PepsimanRenderSystem : public RenderSystem<PepsimanRenderSystemData, PepsimanSystemVertex> {
public:
    PepsimanRenderSystem(std::string pId) : RenderSystem(pId) {}

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes = {};
        poolSizes.uniformBlocksInPool = 3; // 1 for model, 1 for camera, 1 for light
        poolSizes.texturesInPool = 3; // 1 for base texture
        poolSizes.setsInPool = 2; // 1 for model, 1 for camera and light
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
        bindVertexBuffers(commandBuffer);
    }

    void updateUniformBuffers(uint32_t currentImage, PepsimanRenderSystemData data) override {
        PepsimanUniformBufferObject ubo{};
        ubo.model = data.model;
        for (int i = 0; i < Pepsiman_MAX_JOINTS_COUNT; i++) {
            ubo.jointTransformMatrices[i] = data.jointTransformMatrices[i];
        }

        DS.map((int) currentImage, &ubo, MODEL_DATA_BINDING);

        CameraUniformBuffer cameraUBO{};
        cameraUBO.view = camera->matrices.view;
        cameraUBO.projection = camera->matrices.perspective;
        GDS.map((int) currentImage, &cameraUBO, (int) CAMERA_DATA_BINDING);
        LightUnifromBufferObject lightUBO = light->getUBO();
        GDS.map((int) currentImage, &lightUBO, (int) LIGHT_DATA_BINDING);
    }

protected:
    std::string VERT_SHADER = "assets/shaders/bin/pepsiman.vert.spv";
    std::string FRAG_SHADER = "assets/shaders/bin/pepsiman.frag.spv";

    void localCleanup() override {
        P.destroy();
        DSL.cleanup();
        BaseTexture.cleanup();
    }

    void localInit() override {
        VD.init(BP, PepsimanSystemVertex::getBindingDescription(),
                PepsimanSystemVertex::getDescriptorElements());
        GDSL.init(BP, getGDSLBindings());

        DSL.init(BP, {
                {MODEL_DATA_BINDING,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                sizeof(PepsimanUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                       1},
                {METALLIC_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1,                                       1},
                {NORMAL_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2,                                       1},
        });

        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&GDSL, &DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

        initTextures();
        std::cout << "PepsimanRenderSystem initialized" << std::endl;
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