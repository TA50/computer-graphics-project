#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "headers/json.hpp"
#include "common.hpp"
#include "light-object.hpp"
#include "enums.hpp"
#include <iostream>
#include <fstream>
#include <string>

#include "game-objects/game-object-base.hpp"
#include "game-objects/gltf-skin-base.hpp"
#include "game-objects/game-object-loader.hpp"

class SceneLoader {
public:
    std::unordered_map<std::string, RenderType> renderTypes = {
            {"stationary",    STATIONARY},
            {"moving",        MOVING},
            {"animated-skin", ANIMATED_SKIN},
            {"pepsiman",      PEPSIMAN},
            {"metallic",      METTALIC},
    };

    SceneLoader(std::string path) : filename(path) {
        jsonData = nlohmann::json();
    }

    void readJson() {
        std::ifstream i(filename);
        if (!i.is_open()) {
            std::cerr << "Error opening file " << filename << std::endl;
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

    std::unordered_map<std::string, GameObjectBase *> loadGameObjects() {
        if (jsonData.contains("gameObjects")) {
            nlohmann::json gameObjectsData = jsonData["gameObjects"];
            std::unordered_map<std::string, GameObjectBase *> gameObjectsMap;
            for (nlohmann::json::iterator it = gameObjectsData.begin(); it != gameObjectsData.end(); ++it) {
                std::cout << "Loading game object: " << it.key() << std::endl;
                gameObjectsMap[it.key()] = loadGameObject(it.key(), it.value());
            }
            return gameObjectsMap;
        } else {
            return {};
        }
    }

    std::unordered_map<std::string, TextureInfo> loadTextures(nlohmann::json texturesData) {
        std::unordered_map < std::string, TextureInfo > textures;
        for (nlohmann::json::iterator it = texturesData.begin(); it != texturesData.end(); ++it) {
            TextureInfo textureInfo;
            std::string key = it.key();

            auto data = it.value();

            if (!data.contains("path")) {
                throw std::runtime_error("Texture path not found");
            }
            textureInfo.path = std::string(data["path"]);
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
        std::string modelType = gameObjectData["modelType"];
        std::string renderType = gameObjectData["renderType"];
        auto textureInfo = loadTextures(textures);
        for (auto &t: textureInfo) {
            gameObject->addTexture(t.first, t.second);
        }
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
        gameObject->setRenderType(renderTypes[renderType]);
        return gameObject;
    }

    std::unordered_map<std::string, GltfSkinBase *> loadSkins() {
        if (jsonData.contains("skins")) {
            nlohmann::json skinsData = jsonData["skins"];
            std::unordered_map<std::string, GltfSkinBase *> skinsMap;
            for (nlohmann::json::iterator it = skinsData.begin(); it != skinsData.end(); ++it) {
                std::string skinId = it.key();
                std::cout << "Loading Skin: " << skinId << std::endl;
                auto loadedSkin = loadSkin(skinId, it.value());

                skinsMap[skinId] = loadedSkin;
            }
            return skinsMap;
        } else {
            std::cerr << "No skins found in json" << std::endl;
            throw std::runtime_error("No skins found in json");
        }
    }

    GltfSkinBase *loadSkin(const std::string &skinId, nlohmann::json skinData) {

        std::string modelPath = skinData["modelPath"];
        std::string renderType = skinData["renderType"];

        tinygltf::Model model = GltfLoader::loadGlTFFile(modelPath);
        std::vector<GltfSkinBase *> skins{};
        for (const auto &s: model.skins) {
            SkinData data = GltfLoader::loadSkinData(model, s);
            data.id = skinId;
            auto sk = GltfSkinBase::create(data);
            skins.push_back(sk);
        }
        std::cout << "Loaded skin: " << skinId << std::endl;
        GltfSkinBase *skin = skins[0];


        auto textures = skinData["textures"];

        auto textureInfo = loadTextures(textures);
        for (auto &t: textureInfo) {
            skin->addTexture(t.first, t.second);
        }
        skin->setRenderType(renderTypes[renderType]);
        return skin;
    }

    nlohmann::json getJson() {
        return jsonData;
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

            nlohmann::json objectData = jsonData[objectTypeStr][key];

            auto v = glm::vec3(1.0f);
            if (objectData.contains(transformTypeStr)) {
                v.x = jsonData[objectTypeStr][key][transformTypeStr]["x"];
                v.y = jsonData[objectTypeStr][key][transformTypeStr]["y"];
                v.z = jsonData[objectTypeStr][key][transformTypeStr]["z"];
            }

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
        auto pos = glm::vec3(0);
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
        config->heroRotationSpeed = jsonData[key]["heroRotationSpeed"];
    }

    void setLight(Light *light) {
        auto key = "light";
        glm::vec3 translation = glm::vec3(0);
        translation.x = jsonData[key]["translate"]["x"];
        translation.y = jsonData[key]["translate"]["y"];
        translation.z = jsonData[key]["translate"]["z"];

        glm::vec3 rotation = glm::vec3(0);
        if (jsonData[key].contains("rotate")) {
            rotation.x = jsonData[key]["rotate"]["x"];
            rotation.y = jsonData[key]["rotate"]["y"];
            rotation.z = 0;
        }
        auto Q = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (jsonData[key].contains("quaternion")) {
            Q = glm::quat(jsonData[key]["quaternion"][3],
                          jsonData[key]["quaternion"][0],
                          jsonData[key]["quaternion"][1],
                          jsonData[key]["quaternion"][2]);
        }

        glm::vec4 color = glm::vec4(1.0f);
        color.r = jsonData[key]["color"]["r"];
        color.g = jsonData[key]["color"]["g"];
        color.b = jsonData[key]["color"]["b"];
        color.a = jsonData[key]["color"]["a"];
        light->setTranslation(translation);
        light->setColor(color);
        light->setRotation(Q);

        // gamma
        if (jsonData[key].contains("specularGamma")) {
            light->specularGamma = jsonData[key]["specularGamma"];
        }

        // ambient
        if (jsonData[key].contains("ambient")) {
            auto ambient = jsonData[key]["ambient"];
            AmbientColors ambientColors{};
            ambientColors.cxp = glm::vec3(ambient["cxp"]["r"], ambient["cxp"]["g"], ambient["cxp"]["b"]);
            ambientColors.cxn = glm::vec3(ambient["cxn"]["r"], ambient["cxn"]["g"], ambient["cxn"]["b"]);
            ambientColors.cyp = glm::vec3(ambient["cyp"]["r"], ambient["cyp"]["g"], ambient["cyp"]["b"]);
            ambientColors.cyn = glm::vec3(ambient["cyn"]["r"], ambient["cyn"]["g"], ambient["cyn"]["b"]);
            ambientColors.czp = glm::vec3(ambient["czp"]["r"], ambient["czp"]["g"], ambient["czp"]["b"]);
            ambientColors.czn = glm::vec3(ambient["czn"]["r"], ambient["czn"]["g"], ambient["czn"]["b"]);
            light->ambientColors = ambientColors;
        }

        light->update();
    }


private :
    nlohmann::json jsonData;
    std::string filename;
};