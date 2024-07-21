#pragma once

#include "game-objects/game-object-base.hpp"

class MultiMeshGameObject : public GameObjectBase {
public:
    MultiMeshGameObject(std::string pId) :
            GameObjectBase(pId) {
    }

    std::vector<GameObjectBase *> meshes;

    void addMesh(GameObjectBase * mesh) {
        meshes.push_back(mesh);
    }
};