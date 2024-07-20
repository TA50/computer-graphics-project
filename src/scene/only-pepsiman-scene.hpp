#pragma once

#include "scene/scene-base.hpp"
#include "game-objects/game-object-base.hpp"
#include "render-system/stationary-render-system.hpp"
#include "render-system/animated-skin-render-system.hpp"
#include "render-system/pepsiman-render-system.hpp"

class OnlyPepsimanScene : public SceneBase {
public:

    OnlyPepsimanScene(std::string pId, std::string worldFile) :
            SceneBase(pId, worldFile) {
    }

    std::unordered_map<std::string, PepsimanRenderSystem *> pepsimanRenderSystems;

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes;
        for (auto [id, system]: pepsimanRenderSystems) {
            poolSizes.uniformBlocksInPool += system->getPoolSizes().uniformBlocksInPool;
            poolSizes.texturesInPool += system->getPoolSizes().texturesInPool;
            poolSizes.setsInPool += system->getPoolSizes().setsInPool;
        }
        return poolSizes;
    }

    void initRenderSystems() override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->init(BP, camera, light);
        }
    }

    void createRenderSystems() override {
        for (const auto &[id, skin]: skins) {
            if (skin->renderType == PEPSIMAN) {
                auto renderSystem = new PepsimanRenderSystem(id);
                std::vector<PepsimanSystemVertex> vertices;
                for (auto v: skin->vertices) {
                    PepsimanSystemVertex vertex{};
                    vertex.pos = v.pos;
                    vertex.normal = v.normal;
                    vertex.uv = v.uv;
                    vertex.jointIndices = v.jointIndices;
                    vertex.jointWeights = v.jointWeights;
                    vertex.inColor = v.inColor;
                    vertex.tangent = v.tangent;
                    vertices.push_back(vertex);
                }
                renderSystem->addVertices(vertices, skin->indices);
                renderSystem->setTextures(skin->textures);
                pepsimanRenderSystems[id] = renderSystem;
            }
        }
    }


    void localInit() override {
        for (auto [id, s]: skins) {
            s->updateJointMatrices();
        }

    }

    void updateUniformBuffer(uint32_t currentImage, UserInput userInput) override {
        for (auto [id, s]: skins) {
            s->update(BP->frameTime, false);
        }
        if (userInput.key == GLFW_KEY_B) {
            this->sceneLoader.readJson();
            setWorld();
            this->setLight();
        }

        camera->rotate(-userInput.rotation.y * userInput.deltaTime, -userInput.rotation.x * userInput.deltaTime,
                       -userInput.rotation.z * userInput.deltaTime);
        camera->lookAt(skins["pepsiman"]->getPosition());
        camera->updateWorld();
        camera->updateViewMatrix();
        updateRenderSystems(currentImage);
    }

    void updateRenderSystems(uint32_t currentImage) {
        for (auto [id, system]: pepsimanRenderSystems) {
            auto skin = skins[id];
            auto jointMatrices = skin->getJointMatrices();
            PepsimanRenderSystemData data{};
            data.model = skin->getModel();
            for (auto [jointIndex, jointMatrix]: jointMatrices) {
                data.jointTransformMatrices[jointIndex] = jointMatrix;
            }
            system->updateUniformBuffers(currentImage, data);
        }
    }

    void pipelinesAndDescriptorSetsInit() override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
    }

    void localCleanup() override {

        for (auto [id, system]: pepsimanRenderSystems) {
            system->cleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
    }
};