#pragma once
#include "glm/glm.hpp"

struct TextureInfo {
    std::string path;
    bool initSampler = true;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
};


enum RenderType {
    STATIONARY,
    MOVING,
    SKYBOX,
    ANIMATED_SKIN,
    PEPSIMAN,
};


