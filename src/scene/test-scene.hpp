#pragma once

#include "scene/scene-base.hpp"
#include "game-objects/game-object-base.hpp"
#include "render-system/stationary-render-system.hpp"
#include "render-system/animated-skin-render-system.hpp"

class TestScene : public SceneBase {
public:

    TestScene(std::string pId, std::string worldFile) :
            SceneBase(pId, worldFile) {
    }

    std::unordered_map<std::string, StationaryRenderSystem *> stationaryRenderSystems;
    std::unordered_map<std::string, AnimatedSkinRenderSystem *> animatedSkinRenderSystems;

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes;
        for (auto [id, system]: stationaryRenderSystems) {
            poolSizes.uniformBlocksInPool += system->getPoolSizes().uniformBlocksInPool;
            poolSizes.texturesInPool += system->getPoolSizes().texturesInPool;
            poolSizes.setsInPool += system->getPoolSizes().setsInPool;
        }
        for (auto [id, system]: animatedSkinRenderSystems) {
            poolSizes.uniformBlocksInPool += system->getPoolSizes().uniformBlocksInPool;
            poolSizes.texturesInPool += system->getPoolSizes().texturesInPool;
            poolSizes.setsInPool += system->getPoolSizes().setsInPool;
        }
        return poolSizes;
    }

    void initRenderSystems() override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->init(BP, camera, light);
        }
        for (auto [id, system]: animatedSkinRenderSystems) {
            system->init(BP, camera, light);
        }
    }

    void createRenderSystems() override {
        for (auto [id, go]: gameObjects) {
            if (go->renderType == STATIONARY) {
                auto renderSystem = new StationaryRenderSystem(id);
                std::vector<StationarySystemVertex> vertices;

                for (auto v: go->vertices) {
                    StationarySystemVertex vertex;
                    vertex.pos = v.pos;
                    vertex.normal = v.normal;
                    vertex.uv = v.uv;
                    vertices.push_back(vertex);
                }
                renderSystem->addVertices(vertices, go->indices);
                renderSystem->setTextures(go->textures);
                stationaryRenderSystems[id] = renderSystem;
            }

        }

        for (const auto &[id, skin]: skins) {
            if (skin->renderType == ANIMATED_SKIN) {
                auto renderSystem = new AnimatedSkinRenderSystem(id);
                std::vector<AnimatedSkinSystemVertex> vertices;

                for (auto v: skin->vertices) {
                    AnimatedSkinSystemVertex vertex{};
                    vertex.pos = v.pos;
                    vertex.normal = v.normal;
                    vertex.uv = v.uv;
                    vertex.jointIndices = v.jointIndices;
                    vertex.jointWeights = v.jointWeights;
                    vertex.inColor = v.inColor;
                    vertices.push_back(vertex);
                }
                renderSystem->addVertices(vertices, skin->indices);
                renderSystem->setTextures(skin->textures);
                animatedSkinRenderSystems[id] = renderSystem;
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
        camera->rotate(-userInput.rotation.y * userInput.deltaTime, -userInput.rotation.x * userInput.deltaTime, -userInput.rotation.z * userInput.deltaTime);
        camera->lookAt(skins["pepsiman"]->getPosition());
        camera->updateWorld();
        camera->updateViewMatrix();

        updateRenderSystems(currentImage);
    }

    void updateRenderSystems(uint32_t currentImage) {

        for (auto [id, system]: stationaryRenderSystems) {
            auto go = gameObjects[id];
            system->updateUniformBuffers(currentImage, {
                    go->getModel() * camera->matrices.world
            });
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            auto skin = skins[id];
            auto jointMatrices = skin->getJointMatrices();

            AnimatedSkinRenderSystemData data{};
            data.model = skin->getModel();
            for (auto [jointIndex, jointMatrix]: jointMatrices) {
                data.jointTransformMatrices[jointIndex] = jointMatrix;
            }
            system->updateUniformBuffers(currentImage, data);
        }

    }

    void pipelinesAndDescriptorSetsInit() override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }


    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
    }

    void localCleanup() override {

        for (auto [id, system]: stationaryRenderSystems) {
            system->cleanup();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->cleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
    }
};