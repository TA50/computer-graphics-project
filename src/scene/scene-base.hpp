#pragma once

#include <light-object.hpp>
#include <render-system/render-system.hpp>
#include "game-objects/game-object-base.hpp"
#include "modules/Starter.hpp"
#include "camera.hpp"
#include "scene/scene-loader.hpp"

class SceneBase {
public:

    std::unordered_map<std::string, GameObjectBase *> gameObjects;
    Camera camera;
    Light lightObject = Light();
    SceneLoader sceneLoader;
    std::string id;

    SceneBase(std::string pId, std::string worldFile) :
            id(pId),
            sceneLoader(SceneLoader(worldFile)) {

    }

    virtual void localInit() = 0;

    virtual void updateUniformBuffer(uint32_t currentImage) = 0;

    virtual void pipelinesAndDescriptorSetsInit() = 0;

    virtual void pipelinesAndDescriptorSetsCleanup() = 0;

    virtual void localCleanup() = 0;

    virtual void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) = 0;

};
