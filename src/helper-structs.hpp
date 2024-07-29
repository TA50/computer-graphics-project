#pragma once

#include "glm/glm.hpp"
#include "common.hpp"

struct UserInput {
    glm::vec3 axis;
    glm::vec3 rotation;
    int key;
    float deltaTime;
    float aspectRatio;
};

struct skyBoxUniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
};

struct skyBoxVertex {
    glm::vec3 pos;
};


struct GameConfig {
    float heroSpeed = 0.0f;
    float heroRotationSpeed = 0.0f;
    float villainSpeed = 0.0f;
    float heroAnimationSpeed = 1.0f;
    float villainAnimationSpeed = 1.0f;

    void print() {
        std::cout << "Hero Speed: " << heroSpeed << std::endl;
        std::cout << "Can Speed: " << villainSpeed << std::endl;
    }
};

struct LightConfig {
    float ang1 = 0.0f;
    float ang2 = 0.0f;
    float ang3 = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

};


struct GameObjectVertex {
    glm::vec3 pos;
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec2 uv = glm::vec2(0.0f);
    glm::vec4 inColor = glm::vec4(1.0f);
    glm::vec4 tangent = glm::vec4(0.0f);
};




struct SkinVertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 jointIndices;
    glm::vec4 jointWeights;
    glm::vec3 inColor;
    glm::vec4 tangent;
};