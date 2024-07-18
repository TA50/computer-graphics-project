#pragma once

struct LightUnifromBufferObject {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec3 cameraPos;

    static std::vector<DescriptorSetLayoutBinding> getDescriptorSetLayoutBindings() {
        return {
            {
                0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT,
                sizeof(LightUnifromBufferObject), 1
            },
        };
    }
};


class Light {
public:
    Light() = default;

    void init(BaseProject *bp) {
        DSL.init(bp, LightUnifromBufferObject::getDescriptorSetLayoutBindings());
    }



    void setUBO(glm::vec3 lightDir, glm::vec4 lightColor, glm::vec3 eyePos, glm::vec3 cameraPos) {
        lubo.lightDir = lightDir;
        lubo.lightColor = lightColor;
        lubo.eyePos = eyePos;
        lubo.cameraPos = cameraPos;
    }

    LightUnifromBufferObject getUBO() {
        return lubo;
    }

    DescriptorSetLayout getDSL() {
        return DSL;
    }

    DescriptorSet getDS() {
        return DS;
    }

    PoolSizes getPoolSizes() {
        PoolSizes poolSizes = {};
        poolSizes.uniformBlocksInPool = 1;
        poolSizes.texturesInPool = 0;
        poolSizes.setsInPool = 1;
        return poolSizes;
    }


private:
    LightUnifromBufferObject lubo{};
    DescriptorSetLayout DSL;
    DescriptorSet DS;
};
