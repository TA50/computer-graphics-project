#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "headers/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "animated-model/skin.hpp"
#include "game-objects/game-object-base.hpp"
#include "game-objects/game-object-loader.hpp"

enum Transform {
    SCALE,
    ROTATE,
    TRANSLATE
};

enum ObjectType {
    GAME_OBJECT,
    SKIN
};

class SceneLoader {
public:
    SceneLoader(std::string path) : filename(path) {
            jsonData = nlohmann::json();
    }

    void readJson() {
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



    std::unordered_map<std::string, GameObjectBase *> loadGameObjects(Light *lightObject) {
        if (jsonData.contains("gameObjects")) {
            nlohmann::json gameObjectsData = jsonData["gameObjects"];
            std::unordered_map<std::string, GameObjectBase *> gameObjectsMap;
            for (nlohmann::json::iterator it = gameObjectsData.begin(); it != gameObjectsData.end(); ++it) {
                std::cout << "Loading game object: " << it.key() << std::endl;
                gameObjectsMap[it.key()] = loadGameObject(it.key(), it.value());
            }
            return gameObjectsMap;
        } else {
            std::cout << "No game objects found in json" << std::endl;
        }
    }

    std::unordered_map<std::string, TextureInfo> loadTextures(GameObjectBase *gameObject, nlohmann::json texturesData) {
        std::unordered_map < std::string, TextureInfo > textures;
        for (nlohmann::json::iterator it = texturesData.begin(); it != texturesData.end(); ++it) {
            TextureInfo textureInfo;
            std::string key = it.key();

            auto data = it.value();

            if (!data.contains("path")) {
                throw std::runtime_error("Texture path not found");
            }

            textureInfo.path = data["path"];
            if (data.contains("normal")) {
                textureInfo.format = data["normal"] ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
            } else {
                textureInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            }
            if (data.contains("initSampler")) {
                textureInfo.initSampler = data["initSampler"];
            } else {
                textureInfo.initSampler = true;
            }

            textures[key] = textureInfo;
        }

        return textures;
    }

    GameObjectBase *loadGameObject(std::string key, nlohmann::json gameObjectData) {
        auto gameObject = new GameObjectBase(key);
        std::string modelPath = gameObjectData["modelPath"];
        auto textures = gameObjectData["textures"];
        auto textureInfo = loadTextures(gameObject, textures);
        for (auto &t: textureInfo) {
            gameObject->addTexture(t.first, t.second);
        }
        std::string modelType = gameObjectData["modelType"];
        GameObjectLoader::GameObjectLoaderResult result{};
        if (modelType == "gltf") {
            result = GameObjectLoader::loadGltf(modelPath);
        } else if (modelType == "obj") {
            result = GameObjectLoader::loadModelOBJ(modelPath);
        } else {
            throw std::runtime_error("Model type not supported ");
        }

        gameObject->setVertices(result.vertices);
        gameObject->setIndices(result.indices);
        return gameObject;
    }

//    std::unordered_map<std::string, Skin *> loadSkins(Light *lightObject) {
//        if (jsonData.contains("skins")) {
//            nlohmann::json skinsData = jsonData["skins"];
//            std::unordered_map<std::string, Skin *> skinsMap;
//            for (nlohmann::json::iterator it = skinsData.begin(); it != skinsData.end(); ++it) {
//                std::string skinId = it.key();
//                std::cout << "Loading Skin: " << skinId << std::endl;
//                auto skinData = it.value();
//                std::string name = skinData["name"];
//                std::string modelPath = skinData["modelPath"];
//                std::string baseTexture = skinData["baseTexture"];
//                std::string metalicTexture;
//                if (skinData["metallicTexture"].is_string()) {
//                    metalicTexture = static_cast<std::string>(skinData["metallicTexture"]);
//                }
//                std::string diffuseTexture;
//                if (skinData["diffuseTexture"].is_string()) {
//                    diffuseTexture = static_cast<std::string>(skinData["diffuseTexture"]);
//                }
//                std::string roughnessTexture;
//                if (skinData["roughnessTexture"].is_string()) {
//                    roughnessTexture = static_cast<std::string>(skinData["roughnessTexture"]);
//                }
//                Skin *skin;
//                tinygltf::Model model = GltfLoader::loadGlTFFile(modelPath);
//                std::vector<Skin *> skins{};
//                for (const auto &s: model.skins) {
//                    auto sk = new Skin();
//                    GltfLoader::loadSkin(model, s, sk);
//                    skins.push_back(sk);
//                }
//                std::cout << "Loaded skin: " << skinId << std::endl;
//                skin = skins[0];
//                skin->setName(name);
//                skin->setId(skinId);
//                skin->setBaseTexture(baseTexture,
//                                     VK_FORMAT_R8G8B8A8_SRGB,
//                                     true);
//                if (!metalicTexture.empty()) {
//                    skin->setMetalicTexture(metalicTexture, VK_FORMAT_R8G8B8A8_SRGB, true);
//                }
//                if (!diffuseTexture.empty()) {
//                    skin->setDiffuseTexture(diffuseTexture, VK_FORMAT_R8G8B8A8_UNORM, true);
//                }
//                if (!roughnessTexture.empty()) {
//                    skin->setRoughnessTexture(roughnessTexture, VK_FORMAT_R8G8B8A8_SRGB, true);
//                }
//
//                GltfLoader::loadAnimations(skin, model);
//                std::cout << "Loaded animations" << std::endl;
//                bool cameraFollow = skinData["cameraFollow"];
//                if (cameraFollow) {
//                    skin->setCameraFollow(true);
//                } else {
//                    skin->setCameraFollow(false);
//                }
//                skin->setLight(lightObject);
//                skinsMap[skinId] = skin;
//
//            }
//            return skinsMap;
//        } else {
//            std::cerr << "No skins found in json" << std::endl;
//            throw std::runtime_error("No skins found in json");
//        }
//    }


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
            std::cerr << "Error getting transform: " << e.what() << "input: " << " " << objectTypeStr << ":" << ":"
                      << key << ":"
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


    void setLight(LightConfig *config) {
        auto key = "light";
        config->ang1 = jsonData[key]["ang1"];
        config->ang2 = jsonData[key]["ang2"];
        config->ang3 = jsonData[key]["ang3"];
        config->r = jsonData[key]["color"]["r"];
        config->g = jsonData[key]["color"]["g"];
        config->b = jsonData[key]["color"]["b"];
        config->a = jsonData[key]["color"]["a"];
        config->x = jsonData[key]["position"]["x"];
        config->y = jsonData[key]["position"]["y"];
        config->z = jsonData[key]["position"]["z"];
    }

private :
    nlohmann::json jsonData;
    std::string filename;
};