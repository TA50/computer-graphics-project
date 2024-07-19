#pragma once
#include "glm/glm.hpp"
#include "common.hpp"

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



struct GameObjectVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 inColor = glm::vec4(1.0f);

    void setColor(glm::vec4 color) {
        inColor = color;
    }

    static std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, sizeof(GameObjectVertex), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }

    static std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      pos)),     sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32B32_SFLOAT,    static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      normal)),  sizeof(glm::vec3), NORMAL},
                {0, 2, VK_FORMAT_R32G32_SFLOAT,       static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      uv)),      sizeof(glm::vec2), UV},
                {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t >(offsetof(GameObjectVertex,
                                                                                      inColor)), sizeof(glm::vec4), COLOR}
        };
    }
};



struct GameObjectUniformBufferObject : BaseUniformBufferObject {
    GameObjectUniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
            : BaseUniformBufferObject(model, view, projection) {}
};