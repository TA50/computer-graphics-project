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