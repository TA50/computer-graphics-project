#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "animated-model/joint.hpp"
#include "animated-model/animation.hpp"
#include "helper-structs.hpp"

struct SkinData{
    std::string id;
    int rootJointIndex;
    Joint *rootJoint;
    std::unordered_map<int, Joint *> joints;
    std::unordered_map<int, glm::mat4> inverseBindMatrices;
    std::unordered_map<int, glm::mat4> jointMatrices;
    std::vector<SkinVertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<Animation> animations;
    void addJoint(Joint *joint, glm::mat4 inverseBindMatrix, int jointIndex) {
        joints[jointIndex] = joint;
        inverseBindMatrices[jointIndex] = inverseBindMatrix;
        jointMatrices[jointIndex] = joint->getTransformedMatrix();
        joints[jointIndex] = joint;
    }

    void setRootJoint(Joint *joint, int index) {
        rootJointIndex = index;
        rootJoint = joint;
    }
};