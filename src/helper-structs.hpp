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