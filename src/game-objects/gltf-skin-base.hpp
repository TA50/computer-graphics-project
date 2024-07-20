#pragma once

#include "modules/Starter.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "helper-structs.hpp"
#include "animated-model/joint.hpp"
#include "animated-model/animation.hpp"
#include "animated-model/skin-data.hpp"


class GltfSkinBase {
public:

    static GltfSkinBase *create(SkinData data) {
        auto skin = new GltfSkinBase(data.id);
        skin->setVertices(data.vertices);
        skin->setIndices(data.indices);
        skin->setRootJoint(data.rootJoint, data.rootJointIndex);
        for (auto &joint: data.joints) {
            skin->addJoint(joint.second, data.inverseBindMatrices[joint.first], joint.first);
        }
        if (data.animations.empty()) {
            return skin;
        }

        for (auto &animation: data.animations) {
            skin->addAnimation(animation);
        }

        skin->setActiveAnimation(0);
        return skin;

    }

    GltfSkinBase(std::string _id) : id(_id) {
        rootJointIndex = -1;
        joints = std::unordered_map < int, Joint * > ();
        inverseBindMatrices = std::unordered_map < int, glm::mat4 > ();
        jointMatrices = std::unordered_map < int, glm::mat4 > ();
        vertices = std::vector<SkinVertex>();
        indices = std::vector<uint32_t>();
        textures = std::unordered_map < std::string, TextureInfo > ();
    }

    void setRenderType(RenderType type) {
        renderType = type;
    }

    void setVertices(std::vector<SkinVertex> v) {
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

// Transformations
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

    glm::mat4 getLocalMatrix() {
        return LocalMatrix;
    }


    virtual glm::mat4 getModel() {
        return rootJoint->getGlobalMatrix() * getTransformedWorldMatrix();
    }

    glm::vec3 getPosition(){
        auto model = getModel();
        return glm::vec3(model[3]);
    }
    virtual std::unordered_map<int, glm::mat4> getJointMatrices() {
        return jointMatrices;
    }

    glm::mat4 getTransformedWorldMatrix() {
        glm::mat4 M = LocalMatrix;
        M = glm::scale(M, scaling);
        M = glm::rotate(M, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        M = glm::rotate(M, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        M = glm::translate(M, translation);
        return M;
    }

    // skinning
    void setRootJoint(Joint *joint, int index) {
        rootJointIndex = index;
        rootJoint = joint;
    }

    void setActiveAnimation(uint32_t index) {
        activeAnimation = index;
    }


    Joint *getJoint(int index) {
        return joints[index];
    }


    void addJoint(Joint *joint, glm::mat4 inverseBindMatrix, int jointIndex) {
        joints[jointIndex] = joint;
        inverseBindMatrices[jointIndex] = inverseBindMatrix;
        jointMatrices[jointIndex] = joint->getTransformedMatrix();
        joints[jointIndex] = joint;
    }


    void addAnimation(const Animation &animation) {
        animations.push_back(animation);
    }

    void update(float deltaTime, bool pause = true) {
        if (!pause) {
            updateAnimation(deltaTime);
        }
        updateJointMatrices();
    }

    void updateAnimation(float deltaTime) {

        if (activeAnimation > static_cast<uint32_t>(animations.size()) - 1) {
            std::cout << "No animation with index " << activeAnimation << std::endl;
            return;
        }
        auto animation = &animations[activeAnimation];
        animation->currentTime += deltaTime;
        if (animation->currentTime > animation->end) {
            animation->currentTime -= animation->end;
        }

        for (auto &channel: animation->channels) {
            auto sampler = animation->samplers[channel.samplerIndex];
            for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
                if (sampler.interpolation != "LINEAR") {
                    std::cout << "This sample only supports linear interpolations\n";
                    continue;
                }

                // Get the input keyframe values for the current time stamp
                if ((animation->currentTime >= sampler.inputs[i]) &&
                    (animation->currentTime <= sampler.inputs[i + 1])) {
                    float a = (animation->currentTime - sampler.inputs[i]) /
                              (sampler.inputs[i + 1] - sampler.inputs[i]);
                    if (channel.path == "translation") {
                        channel.joint->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
                    }

                    if (channel.path == "rotation") {
                        glm::quat q1;
                        q1.x = sampler.outputsVec4[i].x;
                        q1.y = sampler.outputsVec4[i].y;
                        q1.z = sampler.outputsVec4[i].z;
                        q1.w = sampler.outputsVec4[i].w;

                        glm::quat q2;
                        q2.x = sampler.outputsVec4[i + 1].x;
                        q2.y = sampler.outputsVec4[i + 1].y;
                        q2.z = sampler.outputsVec4[i + 1].z;
                        q2.w = sampler.outputsVec4[i + 1].w;

                        channel.joint->rotation = glm::normalize(glm::slerp(q1, q2, a));
                    }
                    if (channel.path == "scale") {
                        channel.joint->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
                    }
                }
            }
        }
    }

    void updateJointMatrices() {

        auto rootMat = getJointMatrix(rootJoint);

        glm::mat4 inverseTransform = glm::inverse(rootMat);
        for (auto &[index, joint]: joints) {
            // Apply joint transformation and inverse bind matrix
            jointMatrices[index] = getJointMatrix(joint) * inverseBindMatrices[index];
            // Transform back to model space
            jointMatrices[index] = inverseTransform * jointMatrices[index];

        }


    }


    static glm::mat4 getJointMatrix(Joint *joint) {
        glm::mat4 jointMatrix = joint->getTransformedMatrix();
        Joint *currentParent = joint->parent;
        while (currentParent) {
            jointMatrix = currentParent->getTransformedMatrix() * jointMatrix;
            currentParent = currentParent->parent;
        }
        return jointMatrix;
    }

protected:
    std::string id;
    glm::mat4 LocalMatrix = glm::mat4(1.0f);
    glm::vec3 translation = glm::vec3(0.0);
    glm::vec3 scaling = glm::vec3(1.0);
    glm::vec3 rotation = glm::vec3(0.0);
    uint32_t activeAnimation = 0;
    std::vector<Animation> animations;
    int rootJointIndex;
    Joint *rootJoint;
    std::unordered_map<int, Joint *> joints;

    std::unordered_map<int, glm::mat4> inverseBindMatrices;
    std::unordered_map<int, glm::mat4> jointMatrices{};


public:
    std::vector<SkinVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, TextureInfo> textures;
    RenderType renderType = RenderType::ANIMATED_SKIN;
};
