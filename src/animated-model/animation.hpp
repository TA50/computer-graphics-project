#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "animated-model/joint.hpp"

struct AnimationSampler
{
    std::string            interpolation;
    std::vector<float>     inputs;
    std::vector<glm::vec4> outputsVec4;
};

struct AnimationChannel
{
    std::string path;
    Joint *      joint;
    uint32_t    samplerIndex;
};

struct Animation
{
    std::string                   name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float                         start       = std::numeric_limits<float>::max();
    float                         end         = std::numeric_limits<float>::min();
    float                         currentTime = 0.0f;
};