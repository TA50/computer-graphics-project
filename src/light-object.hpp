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
//        float pitch = glm::radians(rotation.x);
//        float yaw = glm::radians(rotation.y);
//        glm::vec3 direction;
//        direction.x = cos(pitch) * cos(yaw);
//        direction.y = sin(pitch);
//        direction.z = cos(pitch) * sin(yaw);
//        lightInfo.direction = glm::normalize(direction);
//        lightInfo.position = translation;
//        lightInfo.color = color;
        LWm = glm::translate(glm::mat4(1), translation) *
              glm::mat4(quat);
        lightInfo.color = color;
        lightInfo.direction = LWm * glm::vec4(0, 1, 0, 0);
        lightInfo.position = LWm * glm::vec4(0, 0, 0, 1);
    }

    void setRotation(glm::quat q) {
        quat = q;
    }
    AmbientColors ambientColors{};
    float specularGamma = 128.0f;
private:
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec4 color;
    glm::quat quat = glm::quat(1, 0, 0, 0);

    glm::mat4 LWm;

};
