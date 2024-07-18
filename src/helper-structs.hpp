#pragma once
#include "glm/glm.hpp"


struct AxisInput{
    glm::vec3 axis;
    glm::vec3 rotation;
    float deltaTime;
};

struct skyBoxUniformBufferObject
{
    alignas(16) glm::mat4 mvpMat;
};

struct skyBoxVertex
{
    glm::vec3 pos;
};


struct GameConfig{
    float heroSpeed = 0.0f;
    float villainSpeed = 0.0f;
    float heroAnimationSpeed = 1.0f;
    float villainAnimationSpeed =1.0f;

    void print(){
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