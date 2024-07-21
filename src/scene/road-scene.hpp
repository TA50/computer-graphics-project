#pragma once

#include "scene/scene-base.hpp"
#include "game-objects/game-object-base.hpp"
#include "render-system/stationary-render-system.hpp"
#include "render-system/animated-skin-render-system.hpp"
#include "render-system/pepsiman-render-system.hpp"

class RoadScene : public SceneBase {
public:

    std::string pepsimanId = "pepsiman";
    std::string followerId = "can";

    RoadScene(std::string pId, std::string worldFile) :
            SceneBase(pId, worldFile) {
    }

    std::unordered_map<std::string, PepsimanRenderSystem *> pepsimanRenderSystems;
    std::unordered_map<std::string, StationaryRenderSystem *> stationaryRenderSystems;

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes;
        for (auto [id, system]: pepsimanRenderSystems) {
            poolSizes.uniformBlocksInPool += system->getPoolSizes().uniformBlocksInPool;
            poolSizes.texturesInPool += system->getPoolSizes().texturesInPool;
            poolSizes.setsInPool += system->getPoolSizes().setsInPool;
        }
        for (auto [id, system]: stationaryRenderSystems) {
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
        for (auto [id, system]: stationaryRenderSystems) {
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
    }


    void localInit() override {
        setWorld();
        setLight();
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
            setCamera(userInput.aspectRatio);
            this->setLight();
        }

        camera->rotate(-userInput.rotation.y * userInput.deltaTime, -userInput.rotation.x * userInput.deltaTime,
                       -userInput.rotation.z * userInput.deltaTime);
        camera->lookAt(skins[pepsimanId]->getPosition());
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

        for (auto [id, system]: stationaryRenderSystems) {
            auto go = gameObjects[id];
            system->updateUniformBuffers(currentImage, {
                    go->getModel()
            });
        }
    }

    void pipelinesAndDescriptorSetsInit() override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }

        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
    }

    void localCleanup() override {

        for (auto [id, system]: pepsimanRenderSystems) {
            system->cleanup();
        }
        for (auto [id, system]: stationaryRenderSystems) {
            system->cleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, system]: pepsimanRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
        for (auto [id, system]: stationaryRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
    }
};