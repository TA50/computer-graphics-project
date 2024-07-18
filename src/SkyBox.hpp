// This has been adapted from the Vulkan tutorial
#pragma once

#include <utility>

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


struct SkyBoxVertex {
    glm::vec3 pos;

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(SkyBoxVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t >(offsetof(SkyBoxVertex,
                                                                                   pos)), sizeof(glm::vec3), POSITION},
        };
    }
};

struct SkyBoxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};


class SkyBox {
public:
    void setBaseTexture(std::string filePath, VkFormat Fmt = VK_FORMAT_R8G8B8A8_SRGB, bool initSampler = true) {
        baseTexturePath = std::move(filePath);
        baseTextureFormat = Fmt;
        baseTextureInitSampler = initSampler;
    }


    PoolSizes getPoolSizes() {
        PoolSizes poolSizes{};
        poolSizes.setsInPool = 1;
        poolSizes.uniformBlocksInPool = 1;
        poolSizes.texturesInPool = 6;
        return poolSizes;
    }

    void setModel(std::string path, ModelType type) {
        modelPath = std::move(path);
        modelType = type;
    }

    void init(BaseProject *bp, Camera *pCamera) {
        this->camera = pCamera;
        this->BP = bp;
        DSL.init(bp, {
                {MVP_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                       sizeof(SkyBoxUniformBufferObject), 1},
                {1,           VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                 1},
        });
        VD.init(bp, SkyBoxVertex::getBindingDescription(), SkyBoxVertex::getDescriptorElements());


        M.init(bp, &VD, modelPath, modelType);
//        BaseTexture.init(BP, baseTexturePath, baseTextureFormat, baseTextureInitSampler);

//        Positive X (right)
//        Negative X (left)
//        Positive Y (top)
//        Negative Y (bottom)
//        Positive Z (front)
//        Negative Z (back)
        BaseTexture.initCubic(BP, {
                "assets/textures/skybox/Citadella2/posx.jpg",
                "assets/textures/skybox/Citadella2/negx.jpg",
                "assets/textures/skybox/Citadella2/posy.jpg",
                "assets/textures/skybox/Citadella2/negy.jpg",
                "assets/textures/skybox/Citadella2/posz.jpg",
                "assets/textures/skybox/Citadella2/negz.jpg",
        }, VK_FORMAT_R8G8B8A8_UNORM);
        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

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
        M.cleanup();
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        P.bind(commandBuffer);
        DS.bind(commandBuffer, P, (int) SET_ID, currentImage);

        M.bind(commandBuffer);

        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M.indices.size()), 1, 0, 0, 0);

    }


    void render(uint32_t currentImage) {
        SkyBoxUniformBufferObject ubo{};
        ubo.mvpMat = camera->matrices.perspective * glm::mat4(glm::mat3(camera->matrices.view));
        DS.map((int) currentImage, &ubo, (int) SET_ID);
    }


private:
    Camera *camera{};
    BaseProject *BP{};


    Pipeline P;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
    DescriptorSet DS;
    DescriptorSetLayout DSL;
    VertexDescriptor VD;

    Texture BaseTexture{};

    VkFormat baseTextureFormat;
    bool baseTextureInitSampler;
    std::string baseTexturePath;

    uint32_t MVP_BINDING = 0;
    uint32_t BASE_TEXTURE_BINDING = 1;

    uint32_t SET_ID = 0;

    Model M;
    std::string modelPath;
    ModelType modelType;

    const std::string VERT_SHADER = "assets/shaders/bin/skybox.vert.spv";
    const std::string FRAG_SHADER = "assets/shaders/bin/skybox.frag.spv";
};
