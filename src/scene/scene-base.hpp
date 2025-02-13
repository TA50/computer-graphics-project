#pragma once

#include <light-object.hpp>
#include <render-system/render-system.hpp>
#include "game-objects/game-object-base.hpp"
#include "modules/Starter.hpp"
#include "camera.hpp"
#include "scene/scene-loader.hpp"

class SceneBase {
public:
    BaseProject *BP;
    std::unordered_map<std::string, GameObjectBase *> gameObjects;
    std::unordered_map<std::string, GltfSkinBase *> skins;
    Camera *camera;
    Light *light;
    SceneLoader sceneLoader;
    std::string id;
    GameConfig gameConfig;


    SceneBase(std::string pId, std::string worldFile) :
            id(pId),
            sceneLoader(SceneLoader(worldFile)) {

    }

    void onWindowResized(float ar) {
        camera->aspect = ar;
        camera->updatePerspective();
    }

    void setCamera(float aspectRatio) {
        sceneLoader.setCamera(camera, aspectRatio);
        camera->updatePerspective();
    }

    void setGame() {
        sceneLoader.setGame(&gameConfig);
    }

    void setLight() {
        sceneLoader.setLight(light);
    }

    void init() {

        this->initRenderSystems();
        setGame();
        this->localInit();
    }

    void load(BaseProject *bp, float ar) {
        this->BP = bp;
        this->sceneLoader.readJson();
        this->camera = new Camera();
        this->light = new Light();
        this->setCamera(ar);
        setLight();
        this->gameObjects = sceneLoader.loadGameObjects();
        this->skins = sceneLoader.loadSkins();

        this->createRenderSystems();
    }


    void setWorld() {
        for (auto [id, go]: gameObjects) {
            auto scale = sceneLoader.get(go->getId(), GAME_OBJECT, SCALE);
            auto rotation = sceneLoader.get(go->getId(), GAME_OBJECT, ROTATE);
            auto translation = sceneLoader.get(go->getId(), GAME_OBJECT, TRANSLATE);
            go->setTranslation(translation);
            go->setRotation(rotation);
            go->setScaling(scale);
        }
        for (auto [id, sk]: skins) {
            auto scale = sceneLoader.get(sk->getId(), SKIN, SCALE);
            auto rotation = sceneLoader.get(sk->getId(), GAME_OBJECT, ROTATE);
            auto translation = sceneLoader.get(sk->getId(), GAME_OBJECT, TRANSLATE);
            sk->setTranslation(translation);
            sk->setRotation(rotation);
            sk->setScaling(scale);
            std::cout << "Skin: " << sk->getId() << std::endl;
        }

        setLight();
    }

    virtual PoolSizes getPoolSizes() = 0;

    virtual void localInit() = 0;

    virtual void initRenderSystems() = 0;

    virtual void createRenderSystems() = 0;

    virtual void updateUniformBuffer(uint32_t currentImage, UserInput userInput) = 0;

    virtual void pipelinesAndDescriptorSetsInit() = 0;

    virtual void pipelinesAndDescriptorSetsCleanup() = 0;

    virtual void localCleanup() = 0;

    virtual void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) = 0;

};
