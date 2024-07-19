#pragma once

#include "glm/glm.hpp"

struct BaseUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 mvp;

    BaseUniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
        this->model = model;
        this->view = view;
        this->modelView = view * model;
        this->mvp = projection * view * model;
    }


private:
    BaseUniformBufferObject() = default;

};


struct ObjectData {
    alignas(16) glm::mat4 model;
};


struct BaseVertex {
    glm::vec3 pos;


    virtual uint32_t getVertexSize() {
        return sizeof(BaseVertex);
    }

    virtual std::vector<VertexDescriptorElement> getDescriptorElements() {
        return {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t >(offsetof(BaseVertex, pos))},
        };

    }

    std::vector<VertexBindingDescriptorElement> getBindingDescription() {
        return {
                {0, this->getVertexSize(), VK_VERTEX_INPUT_RATE_VERTEX},
        };
    }
};

