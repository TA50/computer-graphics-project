#pragma once

#include "modules/Scene.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "headers/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "animated-model/skin.hpp"

enum Transform {
    SCALE,
    ROTATE,
    TRANSLATE
};

enum ObjectType {
    GAME_OBJECT,
    SKIN
};

class WorldLoader {
public:
    WorldLoader(std::string path) : filename(path) {
        jsonData = nlohmann::json();
    }

    void readMatrixJson() {
        std::ifstream i(filename);
        if (!i.is_open()) {
            std::cerr << "Error opening file" << std::endl;
            jsonData = nlohmann::json();
        }

        try {
            nlohmann::json j;
            i >> j;

            i.close();
            jsonData = j;

            std::cout << "Loaded world matrix json" << std::endl;
        }
        catch (nlohmann::json::parse_error &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            throw e;
        }
    }


    std::unordered_map<std::string, GameObject *> loadGameObjects() {
        if (jsonData.contains("gameObjects")) {
            nlohmann::json gameObjectsData = jsonData["gameObjects"];
            std::unordered_map<std::string, GameObject *> gameObjectsMap;
            for (nlohmann::json::iterator it = gameObjectsData.begin(); it != gameObjectsData.end(); ++it) {
                std::cout << "Loading game object: " << it.key() << std::endl;
                auto gameObject = new GameObject(it.key());
                nlohmann::json gameObjectData = it.value();
                std::string modelPath = gameObjectData["modelPath"];
                std::string baseTexture = gameObjectData["baseTexture"];
                std::string modelType = gameObjectData["modelType"];
                GameObjectLoader::GameObjectLoaderResult result{};
                if (modelType == "gltf") {
                    result = GameObjectLoader::loadGltf(modelPath);
                } else if (modelType == "obj") {
                    result = GameObjectLoader::loadModelOBJ(modelPath);
                } else {
                    throw std::runtime_error("Model type not supported ");
                }
                gameObject->setName(gameObjectData["name"]);
                gameObject->setBaseTexture(baseTexture,
                                           VK_FORMAT_R8G8B8A8_UNORM,
                                           true);
                bool cull = gameObjectData["cull"];
                if (!cull) {
                    gameObject->setCullMode(VK_CULL_MODE_NONE);
                }
                gameObject->setVertices(result.vertices);
                gameObject->setIndices(result.indices);
                gameObjectsMap[gameObject->getId()] = gameObject;
            }
            return gameObjectsMap;
        } else {
            std::cerr << "No game objects found in json" << std::endl;
            throw std::runtime_error("No game objects found in json");
        }
    }

    std::unordered_map<std::string, Skin *> loadSkins() {
        if (jsonData.contains("skins")) {
            nlohmann::json skinsData = jsonData["skins"];
            std::unordered_map<std::string, Skin *> skinsMap;
            for (nlohmann::json::iterator it = skinsData.begin(); it != skinsData.end(); ++it) {
                std::string skinId = it.key();
                std::cout << "Loading Skin: " << skinId << std::endl;
                auto skinData = it.value();
                std::string name = skinData["name"];
                std::string modelPath = skinData["modelPath"];
                std::string baseTexture = skinData["baseTexture"];
                Skin *skin;
                tinygltf::Model model = GltfLoader::loadGlTFFile(modelPath);
                std::vector<Skin *> skins{};
                for (const auto &s: model.skins) {
                    auto sk = new Skin();
                    GltfLoader::loadSkin(model, s, sk);
                    skins.push_back(sk);
                }
                std::cout << "Loaded skin: " << skinId << std::endl;
                skin = skins[0];
                skin->setName(name);
                skin->setId(skinId);
                skin->setBaseTexture(baseTexture,
                                     VK_FORMAT_R8G8B8A8_UNORM,
                                     true);
                GltfLoader::loadAnimations(skin, model);
                std::cout << "Loaded animations" << std::endl;
                bool cameraFollow = skinData["cameraFollow"];
                if (cameraFollow) {
                    skin->setCameraFollow(true);
                } else {
                    skin->setCameraFollow(false);
                }

                skinsMap[skinId] = skin;

            }
            return skinsMap;
        } else {
            std::cerr << "No skins found in json" << std::endl;
            throw std::runtime_error("No skins found in json");
        }
    }

    glm::vec3 get(const std::string &key, ObjectType objectType, Transform transformType) {

        std::string transformTypeStr;
        std::string objectTypeStr;
        switch (transformType) {
            case SCALE:
                transformTypeStr = "scale";
                break;
            case ROTATE:
                transformTypeStr = "rotate";
                break;
            case TRANSLATE:
                transformTypeStr = "translate";
                break;
        }
        switch (objectType) {
            case GAME_OBJECT:
                objectTypeStr = "gameObjects";
                break;
            case SKIN:
                objectTypeStr = "skins";
                break;
        }
        try {

            glm::vec3 v = glm::vec3(1.0f);
            v.x = jsonData[objectTypeStr][key][transformTypeStr]["x"];
            v.y = jsonData[objectTypeStr][key][transformTypeStr]["y"];
            v.z = jsonData[objectTypeStr][key][transformTypeStr]["z"];
            return v;
        } catch (nlohmann::json::exception &e) {
            std::cerr << "Error getting transform: " << e.what() << "input: " << " " << objectTypeStr << ":" << ":" << key << ":"
                      << transformTypeStr << std::endl;
            throw e;
        }
    }

    void setCamera(Camera *camera, float aspectRatio) {
        auto key = "camera";
        float yaw = jsonData[key]["euler"]["yaw"]; // z
        float pitch = jsonData[key]["euler"]["pitch"];
        float roll = jsonData[key]["euler"]["roll"];
        camera->setEuler(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
        camera->CamTargetDelta.x = jsonData[key]["targetDelta"]["x"];
        camera->CamTargetDelta.y = jsonData[key]["targetDelta"]["y"];
        camera->CamTargetDelta.z = jsonData[key]["targetDelta"]["z"];
        glm::vec3 pos = glm::vec3(0);
        pos.x = jsonData[key]["position"]["x"];
        pos.y = jsonData[key]["position"]["y"];
        pos.z = jsonData[key]["position"]["z"];
        camera->setPosition(pos);
        float fovDeg = jsonData[key]["perspective"]["fov"];
        camera->fov = glm::radians(fovDeg);
        camera->aspect = aspectRatio;
        camera->znear = jsonData[key]["perspective"]["near"];
        camera->zfar = jsonData[key]["perspective"]["far"];
        camera->CamDistance = jsonData[key]["distance"];
    }


    void setGame(GameConfig *config) {
        auto key = "game";
        config->heroSpeed = jsonData[key]["heroSpeed"];
        config->villainSpeed = jsonData[key]["villainSpeed"];
        config->heroAnimationSpeed = jsonData[key]["heroAnimationSpeed"];
        config->villainAnimationSpeed = jsonData[key]["villainAnimationSpeed"];

    }

private :
    nlohmann::json jsonData;
    std::string filename;
};