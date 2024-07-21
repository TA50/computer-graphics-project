#pragma once

struct AmbientColors {
    alignas(16) glm::vec3 cxp = glm::vec3(1);
    alignas(16) glm::vec3 cxn = glm::vec3(1);
    alignas(16) glm::vec3 cyp = glm::vec3(1);
    alignas(16) glm::vec3 cyn = glm::vec3(1);
    alignas(16) glm::vec3 czp = glm::vec3(1);
    alignas(16) glm::vec3 czn = glm::vec3(1);
};


class Light {
public:

    struct {
        glm::vec3 direction;
        glm::vec3 position;
        glm::vec4 color;
    } lightInfo;


    void setTranslation(glm::vec3 t) {
        translation = t;
    }

    void setRotation(glm::vec3 r) {
        rotation = r;
    }

    void setColor(glm::vec4 c) {
        color = c;
    }

    void update() {
        auto LWm = glm::translate(glm::mat4(1.0f), translation)
                   * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0))
                   * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0))
                   * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));

        lightInfo.direction = LWm * glm::vec4(0, 0, 1, 0);
        lightInfo.position = LWm * glm::vec4(0, 0, 0, 1);
        lightInfo.color = color;
    }

    AmbientColors ambientColors{};
    float specularGamma = 128.0f;
private:
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec4 color;

};
