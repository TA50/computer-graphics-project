// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-object.hpp"
#include "game-objects/game-object-loader.hpp"
#include "world-loader.hpp"
#include "SkyBox.hpp"
#include "render-system/stationary-render-system.hpp"

class RenderSystemsApp : public BaseProject {
protected:

    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    Camera camera;
    std::unordered_map<std::string, GameObject *> gameObjects;
    std::unordered_map<std::string, Skin *> skins;
    StationaryRenderSystem system = StationaryRenderSystem("StationaryRenderSystem");
    GameObject *cola;
    LightConfig lightConfig;


    glm::vec3 CameraInitialPosition = glm::vec3(0, 2.07f, 2);

    WorldLoader worldLoader = WorldLoader("assets/world.json");
    Light lightObject = Light();


    void loadModels(Light *lightObject) {

        gameObjects = worldLoader.loadGameObjects(lightObject);
        std::cout << "Game Objects loaded" << std::endl;
        cola = gameObjects["cola-can"];

    }


    void setWorld() {
        for (auto [id, go]: gameObjects) {
            auto scale = worldLoader.get(go->getId(), GAME_OBJECT, SCALE);
            auto rotation = worldLoader.get(go->getId(), GAME_OBJECT, ROTATE);
            auto translation = worldLoader.get(go->getId(), GAME_OBJECT, TRANSLATE);
            go->setTranslation(translation);
            go->setRotation(rotation);
            go->scale(scale);
        }
        for (auto [id, sk]: skins) {
            auto scale = worldLoader.get(sk->getId(), SKIN, SCALE);
            auto rotation = worldLoader.get(sk->getId(), SKIN, ROTATE);
            auto translation = worldLoader.get(sk->getId(), SKIN, TRANSLATE);
            sk->setTranslation(translation);
            sk->setRotation(rotation);
            sk->setScaling(scale);
            sk->updateWorldMatrix();
        }
    }

    // Here you set the main application parameters
    void setWindowParameters() override {
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "App Name";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};

        Ar = (float) windowWidth / (float) windowHeight;

    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float) w / (float) h;
        camera.aspect = Ar;
        camera.updatePerspective();
    }

    void setLightConfig() {
        worldLoader.setLight(&lightConfig);

    }

    void setCamera() {
        worldLoader.setCamera(&camera, Ar);
        camera.updatePerspective();
    }

    void localInit() {
        worldLoader.readMatrixJson();
        loadModels(&lightObject);
        setCamera();
        setWorld();
        lightObject.init(this);
        auto lightDir = glm::vec3(cos(glm::radians(lightConfig.ang1)) * cos(glm::radians(lightConfig.ang2)),
                                  sin(glm::radians(lightConfig.ang1)),
                                  cos(glm::radians(lightConfig.ang1)) * sin(glm::radians(lightConfig.ang2)));
        auto lightColor = glm::vec4(lightConfig.r, lightConfig.g, lightConfig.b, lightConfig.a);
        auto eyePos = glm::vec3(glm::inverse(camera.matrices.view) * glm::vec4(0, 0, 0, 1));

        lightObject.setUBO(lightDir, lightColor, eyePos, camera.CamPosition);
        std::cout << "Light initialized" << std::endl;


        DPSZs.uniformBlocksInPool += system.getPoolSizes().uniformBlocksInPool;
        DPSZs.texturesInPool += system.getPoolSizes().texturesInPool;
        DPSZs.setsInPool += system.getPoolSizes().setsInPool;

        std::unordered_map < std::string, TextureInfo > textures;
        textures["base"] = {cola->baseTexturePath};
        system.setTextures(textures);
        std::vector<StationarySystemVertex> vertices;
        for (auto &vertex: cola->vertices) {
            StationarySystemVertex v;
            v.normal = vertex.normal;
            v.pos = vertex.pos;
            v.uv = vertex.uv;
            v.inColor = vertex.inColor;

            vertices.push_back(v);
        }
        system.addVertices(vertices, cola->indices);
        system.init(this, &camera, &lightObject);
    }


    void updateUniformBuffer(uint32_t currentImage) override {
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        camera.rotate(-r.y * deltaT, -r.x * deltaT, -r.z * deltaT);

        camera.lookAt(glm::vec3(0,0,0));
        camera.updateWorld();
        camera.updateViewMatrix();


        system.updateUniformBuffers(currentImage, {
                cola->getModel()
        });

    }


    void pipelinesAndDescriptorSetsInit() override {
        system.pipelinesAndDescriptorSetsInit();
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        system.pipelinesAndDescriptorSetsCleanup();
    }


    void localCleanup() override {
        system.cleanup();
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        system.populateCommandBuffer(commandBuffer, currentImage);
    }

};
