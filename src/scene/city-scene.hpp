#pragma once

#include "scene/scene-base.hpp"
#include "game-objects/game-object-base.hpp"
#include "render-system/stationary-render-system.hpp"
#include "render-system/animated-skin-render-system.hpp"
#include "printer.hpp"

/**
 * Dog Animations:
 * 0 - attack
 * 1 - iddle
 * 2 - jump
 * 3 - run
 * 4 - walk
 * 5 - walksent
 */
class CityScene : public SceneBase {
public:

    std::string walkingCharacterId = "walking-character";
    const int ATTACK_ANIMATION = 0;
    const int IDLE_ANIMATION = 1;
    const int JUMP_ANIMATION = 2;
    const int RUN_ANIMATION = 3;
    const int WALK_ANIMATION = 4;
    const int WALKSENT_ANIMATION = 5;

    CityScene(std::string pId, std::string worldFile) :
            SceneBase(pId, worldFile) {
    }

    glm::mat4 CityWorldMatrix = glm::mat4(1.0f);
    std::vector<GameObjectBase *> cityMeshes = {

    };
    std::unordered_map<std::string, StationaryRenderSystem *> cityRenderSystems;
    std::unordered_map<std::string, AnimatedSkinRenderSystem *> animatedSkinRenderSystems;

    PoolSizes getPoolSizes() override {
        PoolSizes poolSizes;
        for (auto [id, system]: cityRenderSystems) {
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
        for (auto [id, system]: cityRenderSystems) {
            system->init(BP, camera, light);
        }
        for (auto [id, system]: animatedSkinRenderSystems) {
            system->init(BP, camera, light);
        }
    }

    void setCity() {
        auto json = sceneLoader.getJson();
        nlohmann::json j = json["city"];
        glm::vec3 scale = glm::vec3(1);
        glm::vec3 pos = glm::vec3(0);
        glm::vec3 rot = glm::vec3(0);
        if (j.contains("scale")) {
            nlohmann::json s = j["scale"];
            float x = s["x"];
            float y = s["y"];
            float z = s["z"];
            scale = glm::vec3(x, y, z);
        }
        if (j.contains("translate")) {
            nlohmann::json s = j["translate"];
            float x = s["x"];
            float y = s["y"];
            float z = s["z"];
            pos = glm::vec3(x, y, z);
        }
        if (j.contains("rotate")) {
            nlohmann::json s = j["rotate"];
            float x = s["x"];
            float y = s["y"];
            float z = s["z"];
            rot = glm::vec3(x, y, z);
        }

        CityWorldMatrix = glm::translate(glm::mat4(1), pos)
                          * glm::rotate(glm::mat4(1), glm::radians(rot.y), glm::vec3(0, 1, 0))
                          * glm::rotate(glm::mat4(1), glm::radians(rot.x), glm::vec3(1, 0, 0))
                          * glm::rotate(glm::mat4(1), glm::radians(rot.z), glm::vec3(0, 0, 1))
                          * glm::scale(glm::mat4(1), scale);

    }

    void createRenderSystems() override {
        auto json = sceneLoader.getJson();
        nlohmann::json j = json["city"];
        std::string baseFolder = j["modelFolder"];
        std::string modelPath = j["modelPath"];
        auto model = GameObjectLoader::loadGltfMulti(modelPath);
        CityWorldMatrix = model.Wm;
        for (auto m: model.meshes) {

            auto go = new GameObjectBase(m.name);
            go->vertices = m.vertices;
            go->indices = m.indices;
            TextureInfo textureInfo{};
            textureInfo.path = baseFolder + "/" + m.baseColorTexture;
            textureInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            textureInfo.initSampler = true;

            go->addTexture("base", textureInfo);
            go->renderType = STATIONARY;
            gameObjects[m.name] = go;

            auto renderSystem = new StationaryRenderSystem(go->getId());
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
            cityRenderSystems[go->getId()] = renderSystem;
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
                cityRenderSystems[id] = renderSystem;
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


        setCity();

    }

    bool pause = false;
    float lookAng = 180;

    void updateUniformBuffer(uint32_t currentImage, UserInput userInput) override {
        auto walkingCharacter = skins[walkingCharacterId];
        walkingCharacter->setActiveAnimation(IDLE_ANIMATION);

        if (userInput.key == GLFW_KEY_P) {
            pause = true;
        }

        if (userInput.key == GLFW_KEY_O) {
            pause = false;
        }

        if (userInput.key == GLFW_KEY_0) {
            walkingCharacter->setTranslation(glm::vec3(0, 0, 0));
            walkingCharacter->setRotation(glm::vec3(0, 0, 0));
        }
        if (!pause) {
            if (userInput.key == GLFW_KEY_X) {
                walkingCharacter->setActiveAnimation(JUMP_ANIMATION);
            } else if (userInput.key == GLFW_KEY_C) {
                walkingCharacter->setActiveAnimation(ATTACK_ANIMATION);
            } else {


                auto m = glm::vec3(1);
                m.x = userInput.axis.x;
                m.y = 0;
                m.z = userInput.axis.z;

                lookAng -= m.x * gameConfig.heroRotationSpeed * userInput.deltaTime;
                auto movingDirection = glm::vec3(0, 0, m.z);

                auto R = glm::rotate(glm::mat4(1), glm::radians(lookAng), glm::vec3(0, 1, 0));
                auto transformedMovingDir = R * glm::vec4(movingDirection, 1);
                movingDirection.x = transformedMovingDir.x;
                movingDirection.y = transformedMovingDir.y;
                movingDirection.z = transformedMovingDir.z;
                walkingCharacter->setRotation(glm::vec3(0, lookAng, 0));
                if (m.z != 0 || m.x != 0) {
                    walkingCharacter->setActiveAnimation(WALK_ANIMATION);
                    walkingCharacter->move(movingDirection * gameConfig.heroSpeed * userInput.deltaTime);
                }

            }
        }

        for (auto [id, s]: skins) {
            s->update(BP->frameTime * gameConfig.heroAnimationSpeed, pause);
        }

        if (userInput.key == GLFW_KEY_B) {
            this->sceneLoader.readJson();
//            setWorld();
            this->setLight();
            setCity();
            setCamera(userInput.aspectRatio);

            setGame();
        }

        camera->rotate(-userInput.rotation.y * userInput.deltaTime, -userInput.rotation.x * userInput.deltaTime,
                       -userInput.rotation.z * userInput.deltaTime);
        camera->lookAt(walkingCharacter->getPosition());
        camera->updateWorld();
        camera->updateViewMatrix();
        updateRenderSystems(currentImage);
    }

    void updateRenderSystems(uint32_t currentImage) {

        for (auto [id, system]: cityRenderSystems) {
            auto go = gameObjects[id];
            system->updateUniformBuffers(currentImage, {
                    CityWorldMatrix * go->getModel()
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
        for (auto [id, system]: cityRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->pipelinesAndDescriptorSetsInit();
        }


    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, system]: cityRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->pipelinesAndDescriptorSetsCleanup();
        }
    }

    void localCleanup() override {

        for (auto [id, system]: cityRenderSystems) {
            system->cleanup();
        }

        for (auto [id, system]: animatedSkinRenderSystems) {
            system->cleanup();
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, system]: cityRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }


        for (auto [id, system]: animatedSkinRenderSystems) {
            system->populateCommandBuffer(commandBuffer, currentImage);
        }
    }
};