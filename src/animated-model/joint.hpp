#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <string>
#include <vector>
#include "cassert"

struct Joint {
public:
    Joint *parent{};
    std::vector<Joint *> children;
    glm::vec3 scale{};
    glm::quat rotation{};
    glm::vec3 translation{};

    explicit Joint(std::string name, int index) : name(std::move(name)),  index(index) {
        scale = glm::vec3(1);
        rotation = glm::quat(1, 0, 0, 0);
        translation = glm::vec3(0);
        localMatrix = glm::mat4(1);
    }


    void updateTransformedMatrix() {
        transformedMatrix = glm::scale(localMatrix, scale);
        transformedMatrix = glm::mat4_cast(rotation) * transformedMatrix;
        transformedMatrix = glm::translate(transformedMatrix, translation);
    }

    glm::mat4 &getTransformedMatrix() {
        return transformedMatrix;
    }

    glm::mat4 &getLocalMatrix() {
        return localMatrix;
    }
    glm::mat4 getGlobalMatrix() {
        if (parent) {
            return parent->getGlobalMatrix() * transformedMatrix;
        } else {
            return localMatrix;
        }
    }
    void setLocalMatrix(glm::mat4 m) {
        assert(!localMatrixSet);
        localMatrix = m;
        localMatrixSet = true;
    }
    void setLocalMatrix(){
        assert(!localMatrixSet);
        localMatrix = glm::translate(glm::mat4(1), translation);
        localMatrix = glm::mat4_cast(rotation) * localMatrix;
        localMatrix = glm::scale(localMatrix, scale);
        localMatrixSet = true;
    }

    std::string getName(){
        return name;
    }

    int getIndex(){
        return index;
    }



private:
    int index{};
    std::string name;
    glm::mat4 transformedMatrix{};
    glm::mat4 localMatrix{};
    bool localMatrixSet = false;
};
