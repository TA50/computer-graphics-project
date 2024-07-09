#pragma once

#include "modules/Starter.hpp"


class AnimatedModel{

public:
    void init(std::string filename){}

    void pipelinesAndDescriptorSetsInit()
    {}

    void pipelinesAndDescriptorSetsCleanup()
    {}

    void localCleanup()
    {}

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
    {}

    void updateUniformBuffer(uint32_t currentImage)
    {}


protected:
    BaseProject * BP;

    DescriptorSet DS;
    Pipeline P;
    DescriptorSetLayout DSL;

//    Texture T;

};