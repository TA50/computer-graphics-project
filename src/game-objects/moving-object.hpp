#pragma once

#include "game-objects/game-object-base.hpp"

class MovingObject : public GameObjectBase {
    void move(glm::vec3 direction) {
        translation += direction;
    }
    void scale(glm::vec3 s) {
        scaling = s;
    }
    void rotate(glm::vec3 degrees) {
        rotation += degrees;
    }
};