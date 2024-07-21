// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


class GameObjectBase {
public:
    GameObjectBase(std::string _id) : id(_id) {

    }

    void setRenderType(RenderType type) {
        renderType = type;
    }

    void setVertices(std::vector<GameObjectVertex> v) {
        vertices = v;
    }

    void setIndices(std::vector<uint32_t> i) {
        indices = i;
    }

    void addTexture(std::string key, TextureInfo textureInfo) {
        textures[key] = textureInfo;
    }

    TextureInfo getTexture(std::string key) {
        return textures[key];
    }

    std::string getId() {
        return id;
    }


    void setTranslation(glm::vec3 pos) {
        translation = pos;
    }

    void setScaling(glm::vec3 s) {
        scaling = s;
    }

    void setRotation(glm::vec3 degrees) {
        rotation = degrees;
    }

    void setLocalMatrix(glm::mat4 m) {
        LocalMatrix = m;
    }

    glm::vec3 getPosition() {
        auto model = getModel();
        return glm::vec3(model[3]);
    }

    glm::mat4 getLocalMatrix() {
        return LocalMatrix;
    }

    virtual glm::mat4 getModel() {
        glm::mat4 M = LocalMatrix;
        auto S = glm::scale(glm::mat4(1), scaling);
        auto T = glm::translate(glm::mat4(1), translation);
        auto Ry = glm::rotate(glm::mat4(1), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        auto Rx = glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        auto Rz = glm::rotate(glm::mat4(1), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        return M * T * Ry * Rx * Rz * S;
    }

    void move(glm::vec3 direction) {
        translation += direction;
    }
    void scale(glm::vec3 s) {
        scaling = s;
    }
    void rotate(glm::vec3 degrees) {
        rotation += degrees;
    }
    std::vector<GameObjectVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, TextureInfo> textures;
    RenderType renderType = RenderType::STATIONARY;
protected:
    std::string id;
    glm::mat4 LocalMatrix = glm::mat4(1.0f);
    glm::vec3 translation = glm::vec3(0.0);
    glm::vec3 scaling = glm::vec3(1.0);
    glm::vec3 rotation = glm::vec3(0.0);

};
