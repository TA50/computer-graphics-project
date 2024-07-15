#pragma once

#include "modules/Scene.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "headers/json.hpp"
#include <iostream>
#include <fstream>
#include <string>


enum Transform {
    SCALE,
    ROTATE,
    TRANSLATE
};

class WorldLoader {
public:
    WorldLoader(std::string path) : filename(path) {
        matrixJson = nlohmann::json();
    }

    void readMatrixJson() {
        std::ifstream i(filename);
        if (!i.is_open()) {
            std::cerr << "Error opening file" << std::endl;
            matrixJson = nlohmann::json();
        }

        try {
            nlohmann::json j;
            i >> j;

            i.close();
            matrixJson = j;

            std::cout << "Loaded world matrix json" << std::endl;
        }
        catch (nlohmann::json::parse_error &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            throw e;
        }
    }

    glm::mat4 getMatrix(std::string key) {
        glm::mat4 M = glm::mat4(1.0f);
        float scaleX = matrixJson[key]["scale"]["x"];
        float scaleY = matrixJson[key]["scale"]["y"];
        float scaleZ = matrixJson[key]["scale"]["z"];

        float rotX = matrixJson[key]["rotate"]["x"];
        float rotY = matrixJson[key]["rotate"]["y"];
        float rotZ = matrixJson[key]["rotate"]["z"];

        float transX = matrixJson[key]["translate"]["x"];
        float transY = matrixJson[key]["translate"]["y"];
        float transZ = matrixJson[key]["translate"]["z"];

        M = glm::scale(M, glm::vec3(scaleX, scaleY, scaleZ));
        M = glm::rotate(M, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

        M = glm::translate(M, glm::vec3(transX, transY, transZ));
        std::cout << "Getting world matrix for " << key << std::endl;
        return M;
    }

    glm::vec3 get(std::string key, Transform type) {
        std::string typeStr;
        switch (type) {
            case SCALE:
                typeStr = "scale";
                break;
            case ROTATE:
                typeStr = "rotate";
                break;
            case TRANSLATE:
                typeStr = "translate";
                break;
        }
        glm::vec3 v = glm::vec3(1.0f);
        v.x = matrixJson[key][typeStr]["x"];
        v.y = matrixJson[key][typeStr]["y"];
        v.z = matrixJson[key][typeStr]["z"];
        return v;
    }

    void setCamera(Camera *camera, float aspectRatio) {
        auto key = "camera";
        float yaw = matrixJson[key]["euler"]["yaw"]; // z
        float pitch = matrixJson[key]["euler"]["pitch"];
        float roll = matrixJson[key]["euler"]["roll"];
        camera->setEuler(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
        camera->CamTargetDelta.x = matrixJson[key]["targetDelta"]["x"];
        camera->CamTargetDelta.y = matrixJson[key]["targetDelta"]["y"];
        camera->CamTargetDelta.z = matrixJson[key]["targetDelta"]["z"];
        glm::vec3 pos = glm::vec3(0);
        pos.x = matrixJson[key]["position"]["x"];
        pos.y = matrixJson[key]["position"]["y"];
        pos.z = matrixJson[key]["position"]["z"];
        camera->setPosition(pos);
        float fovDeg = matrixJson[key]["perspective"]["fov"];
        camera->fov = glm::radians(fovDeg);
        camera->aspect = aspectRatio;
        camera->znear = matrixJson[key]["perspective"]["near"];
        camera->zfar = matrixJson[key]["perspective"]["far"];
        camera->CamDistance = matrixJson[key]["distance"];

    }



    void setGame(GameConfig *config) {
        auto key = "game";
        config->heroSpeed = matrixJson[key]["heroSpeed"];
        config->villainSpeed = matrixJson[key]["villainSpeed"];
        config->heroAnimationSpeed = matrixJson[key]["heroAnimationSpeed"];
        config->villainAnimationSpeed = matrixJson[key]["villainAnimationSpeed"];

    }

private :
    nlohmann::json matrixJson;
    std::string filename;
};