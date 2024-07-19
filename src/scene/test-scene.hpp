#pragma once

#include "scene/scene-base.hpp"
#include "game-objects/game-object-base.hpp"
#include "render-system/stationary-render-system.hpp"

class TestScene : public SceneBase {
public:

    TestScene(std::string pId, std::string worldFile) :
            SceneBase(pId, worldFile) {
    }

    std::unordered_map<std::string, StationaryRenderSystem *> stationaryRenderSystems;

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes;
        for (auto [id, system]: stationaryRenderSystems) {
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
    }


    void localInit() override {
    }

    void updateUniformBuffer(uint32_t currentImage) override {


        camera->lookAt(glm::vec3(0, 0, 0));
        camera->updateWorld();
        camera->updateViewMatrix();


        for (auto [id, system]: stationaryRenderSystems) {
            auto go = gameObjects[id];
            system->updateUniformBuffers(currentImage, {
                    go->getModel()
            });
        }
    }

    void pipelinesAndDescriptorSetsInit() override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }


    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
    }

    void localCleanup() override {

        for (auto [id, system]: stationaryRenderSystems) {
            system->cleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, system]: stationaryRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
    }
};