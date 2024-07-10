#pragma once

#include "glm/glm.hpp"

struct BaselUniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 mvp;

    BaselUniformBufferObject(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
        this->model = model;
        this->view = view;
        this->modelView = view * model;
        this->mvp = projection * view * model;
    }


private:
    BaselUniformBufferObject() = default;

};

