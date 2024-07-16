#pragma once
#include "modules/Starter.hpp"
#include "game-object.hpp"

class Scene{
    void setName(std::string n) {
        name = n;
    }

    void setWorldMatrix(glm::mat4 W) {
        Wm = W;
    }

    void setCullMode(VkCullModeFlagBits flag) {
        this->cullMode = flag;
    }

    void setVertices(std::vector<GameObjectVertex> v) {
        vertices = v;
    }

    void setIndices(std::vector<uint32_t> i) {
        indices = i;
    }

    void setModel(std::string filePath, ModelType type) {
        modelPath = filePath;
        modelType = type;
        loaded = true;
    }

    void setBaseTexture(std::string filePath, VkFormat Fmt, bool initSampler = false) {
        baseTexturePath = filePath;
        baseTextureFormat = Fmt;
        baseTextureInitSampler = initSampler;

    }

    std::string getId() {
        return id;
    }

    PoolSizes getPoolSizes() {
        PoolSizes poolSizes{};
        poolSizes.setsInPool = 1;
        poolSizes.uniformBlocksInPool = 1;
        poolSizes.texturesInPool = 1;
        return poolSizes;
    }

    void init(BaseProject *bp, Camera *pCamera) {
        this->camera = pCamera;
        this->BP = bp;
        DSL.init(bp, {
                {MVP_BINDING,          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                                                sizeof(GameObjectUniformBufferObject), 1},
                {BASE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,                                     1},
        });
        VD.init(bp, GameObjectVertex::getBindingDescription(),
                GameObjectVertex::getDescriptorElements());

        BaseTexture.init(BP, baseTexturePath, baseTextureFormat, baseTextureInitSampler);


        P.init(BP, &VD, VERT_SHADER, FRAG_SHADER, {&DSL});
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
                              cullMode, false);

        if (loaded) {
            M.init(bp, &VD, modelPath, modelType);
        } else {
            createVertexBuffer();
            createIndexBuffer();
        }
    }


    void pipelinesAndDescriptorSetsInit() {
        P.create();
        DS.init(BP, &DSL, {&BaseTexture});
    }

    void pipelinesAndDescriptorSetsCleanup() {
        DS.cleanup();
        P.cleanup();
    }


    void localCleanup() {
        P.destroy();
        VD.cleanup();
        DSL.cleanup();
        BaseTexture.cleanup();
        if (loaded) {
            M.cleanup();
        } else {
            vkDestroyBuffer(BP->device, vertexBuffer, nullptr);
            vkFreeMemory(BP->device, vertexBufferMemory, nullptr);

            vkDestroyBuffer(BP->device, indexBuffer, nullptr);
            vkFreeMemory(BP->device, indexBufferMemory, nullptr);
        }
    }

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        P.bind(commandBuffer);
        DS.bind(commandBuffer, P, SET_ID, currentImage);
        if (loaded) {
            M.bind(commandBuffer);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(M.indices.size()), 1, 0, 0, 0);

        } else {
            VkBuffer vertexBuffers[] = {vertexBuffer};
            // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0,
                                 VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }

    }


    void render(uint32_t currentImage) {

        auto model = getModel();

        GameObjectUniformBufferObject ubo = GameObjectUniformBufferObject(model,
                                                                          camera->matrices.view,
                                                                          camera->matrices.perspective);


        DS.map((int) currentImage, &ubo, (int) SET_ID);
    }


    void setTranslation(glm::vec3 pos) {
        translation = pos;
    }

    void move(glm::vec3 pos) {
        translation += pos;
    }

    void scale(glm::vec3 scale) {
        scaling = scale;
    }

    void setRotation(glm::vec3 degrees) {
        rotation = degrees;
    }

    void rotate(glm::vec3 degrees) {
        rotation += degrees;
    }

    void setLocalMatrix(glm::mat4 m) {
        LocalMatrix = m;
    }

    glm::mat4 getLocalMatrix() {
        return LocalMatrix;
    }

    void updateWorld() {
        // scale
        std::cout << name << std::endl;
        std::cout << "Pre" << std::endl;
        Printer::printArrArr(Wm);
        Wm = glm::scale(LocalMatrix, scaling);
        std::cout << "scale x " << scaling.x << " y " << scaling.y << " z " << scaling.z << std::endl;
        Wm = glm::rotate(Wm, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        Wm = glm::rotate(Wm, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        Wm = glm::translate(Wm, translation);
        std::cout << "After" << std::endl;
        Printer::printArrArr(Wm);

        rotation = glm::vec3(0.0);
        translation = glm::vec3(0.0);
        scaling = glm::vec3(1.0);
    }


    glm::mat4 getModel() {
        // scale
        auto M = LocalMatrix;
//        glm::mat4 model = glm::translate(M, -translation);

        auto model = glm::scale(M, scaling);

        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate
        model = glm::translate(model, translation);

        return model * OriginMatrix;
    }

    std::string getName() {
        return name;

    }


    std::vector<glm::vec3> getVertexLocations() {
        std::vector<glm::vec3> vertexLocations{vertices.size()};
        for (const auto &vertex: vertices) {
            vertexLocations.push_back(vertex.pos);
        }
        return vertexLocations;
    }

    glm::vec3 calculateGeometricCenter() {
        std::vector<glm::vec3> vertexLocations = getVertexLocations();
        glm::vec3 center(0.0f);
        for (const auto &vertex: vertexLocations) {
            center += vertex;
        }
        center /= static_cast<float>(vertexLocations.size());
        return center;
    }

    void changeCenterToGeometricCenter() {
        glm::vec3 geometricCenter = calculateGeometricCenter();
        changeModelCenter(geometricCenter);
    }

    void changeModelCenter(const glm::vec3 &newCenter) {
        glm::vec3 currentCenter = calculateGeometricCenter();
        glm::vec3 translationToOrigin = -currentCenter;
        glm::vec3 translationToNewCenter = newCenter;

        glm::mat4 translateToOriginMatrix = glm::translate(glm::mat4(1.0f), translationToOrigin);
        glm::mat4 translateToNewCenterMatrix = glm::translate(glm::mat4(1.0f), translationToNewCenter);

        // Combined transformation matrix: move to origin, then move to new center
        OriginMatrix = translateToNewCenterMatrix;
    }

private:
    std::string name;



    bool loaded = false;


};