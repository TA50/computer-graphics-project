// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-object.hpp"
#include "game-objects/game-object-loader.hpp"
#include "world-loader.hpp"
#include "light-object.hpp"


class App : public BaseProject {
protected:
    const std::string pepsimanId = "pepsiman";
    const std::string lunaId = "luna";
    Skin *pepsiman;
    Skin *luna;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    float Ar;

    GameConfig gameConfig;
    LightConfig lightConfig;


    WorldLoader worldLoader = WorldLoader("assets/world.json");

    Camera camera;

    std::unordered_map<std::string, GameObject *> gameObjects;
    std::unordered_map<std::string, Skin *> skins;

    glm::vec3 CameraInitialPosition = glm::vec3(0, 2.07f, 2);

    Light lightObject = Light();


    // Here you set the main application parameters
    void setWindowParameters() override {
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "App Name";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.001f, 0.001f, 0.001f, 1.0f};


        Ar = (float) windowWidth / (float) windowHeight;

    }
    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float) w / (float) h;
        camera.aspect = Ar;
        camera.updatePerspective();
    }


    void loadModels(Light *lightObject) {

        gameObjects = worldLoader.loadGameObjects(lightObject);
        std::cout << "Game Objects loaded" << std::endl;
        skins = worldLoader.loadSkins(lightObject);
        std::cout << "Skins loaded" << std::endl;
        pepsiman = skins[pepsimanId];
        luna = skins[lunaId];
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

    void setLightConfig() {
        worldLoader.setLight(&lightConfig);
    }

    void setGameConfig() {
        worldLoader.setGame(&gameConfig);
    }

    void setCamera() {
        worldLoader.setCamera(&camera, Ar);
        camera.updatePerspective();
    }


    void localInit() {
        worldLoader.readMatrixJson();

        std::cout << "Light initialized" << std::endl;
        setLightConfig();
        lightObject.init(this);
        auto lightDir = glm::vec3(cos(glm::radians(lightConfig.ang1)) * cos(glm::radians(lightConfig.ang2)), sin(glm::radians(lightConfig.ang1)),
                                  cos(glm::radians(lightConfig.ang1)) * sin(glm::radians(lightConfig.ang2)));
        auto lightColor = glm::vec4(lightConfig.r, lightConfig.g, lightConfig.b, lightConfig.a);
        auto eyePos = glm::vec3(glm::inverse(camera.matrices.view) * glm::vec4(0, 0, 0, 1));

        lightObject.setUBO(lightDir, lightColor, eyePos, camera.CamPosition);

        loadModels(&lightObject);
        setCamera();
        setWorld();
        setGameConfig();


        for (auto [id, go]: gameObjects) {
            DPSZs.uniformBlocksInPool += go->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += go->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += go->getPoolSizes().setsInPool;

            DPSZs.uniformBlocksInPool += lightObject.getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += lightObject.getPoolSizes().texturesInPool;
            DPSZs.setsInPool += lightObject.getPoolSizes().setsInPool;

            go->init(this, &camera);
        }
        for (auto [id, sk]: skins) {
            DPSZs.uniformBlocksInPool += sk->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += sk->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += sk->getPoolSizes().setsInPool;

            DPSZs.uniformBlocksInPool += lightObject.getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += lightObject.getPoolSizes().texturesInPool;
            DPSZs.setsInPool += lightObject.getPoolSizes().setsInPool;

            sk->init(this, &camera);
        }

        std::cout << "Model initialized" << std::endl;
    }


    bool pause = true;

    void updateUniformBuffer(uint32_t currentImage) override {
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);


        AxisInput axisInput{};
        axisInput.axis = m;
        axisInput.rotation = r;
        axisInput.deltaTime = deltaT;


        if (glfwGetKey(window, GLFW_KEY_0)) {
            // RESET
            pepsiman->setTranslation(glm::vec3(0));
            luna->setTranslation(glm::vec3(0));
        }
        if (glfwGetKey(window, GLFW_KEY_P)) {
            pause = false;
        }
        if (glfwGetKey(window, GLFW_KEY_O)) {
            pause = true;
        }
        if (glfwGetKey(window, GLFW_KEY_B)) {
            worldLoader.readMatrixJson();
            setLightConfig();
            auto lightDir = glm::vec3(cos(glm::radians(lightConfig.ang1)) * cos(glm::radians(lightConfig.ang2)), sin(glm::radians(lightConfig.ang1)),
                                              cos(glm::radians(lightConfig.ang1)) * sin(glm::radians(lightConfig.ang2)));
            auto lightColor = glm::vec4(lightConfig.r, lightConfig.g, lightConfig.b, lightConfig.a);
            auto eyePos = glm::vec3(glm::inverse(camera.matrices.view) * glm::vec4(lightConfig.x, lightConfig.y, lightConfig.z, 1));
            lightObject.setUBO(lightDir, lightColor, eyePos, camera.CamPosition);
            setWorld();
            setCamera();
            setGameConfig();
            std::cout << "Reset World" << std::endl;

        }



        auto Rz = glm::rotate(glm::mat4(1), glm::radians(10.0f), glm::vec3(1, 0, 0));
        auto T = glm::translate(glm::mat4(1), glm::vec3(0.1, gameConfig.villainSpeed, 0));


        if (!pause) {
            pepsiman->move(glm::vec3(0, -gameConfig.heroSpeed, 0));
            pepsiman->updateAnimation(frameTime * gameConfig.heroAnimationSpeed);
            luna->move(glm::vec3(0, -gameConfig.villainSpeed, 0));
            luna->updateAnimation(frameTime * gameConfig.villainAnimationSpeed);
        }

        camera.rotate(-r.y * deltaT, -r.x * deltaT, -r.z * deltaT);

        if (glfwGetKey(window, GLFW_KEY_V)) {
            camera.print();
            auto pos = pepsiman->getPosition();
            std::cout << "Skin Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
            gameConfig.print();
        }

        camera.lookAt(pepsiman->getPosition());
        camera.updateWorld();
        camera.updateViewMatrix();


        for (auto [id, sk]: skins) {
            sk->render(currentImage);
        }
        for (auto [id, go]: gameObjects) {
            go->render(currentImage);
        }

    }


    void pipelinesAndDescriptorSetsInit() override {
        for (auto [id, sk]: skins) {
            sk->createPipelineAndDescriptorSets();
        }

        for (auto [id, go]: gameObjects) {
            go->pipelinesAndDescriptorSetsInit();
        }
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [id, sk]: skins) {
            sk->pipelinesAndDescriptorSetsCleanup();
        }

        for (auto [id, go]: gameObjects) {
            go->pipelinesAndDescriptorSetsCleanup();
        }
    }


    void localCleanup() override {

        for (auto [id, sk]: skins) {
            sk->localCleanup();
        }
        for (auto [id, go]: gameObjects) {
            go->localCleanup();
        }
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        for (auto [id, sk]: skins) {
            sk->bind(commandBuffer, currentImage);
        }
        for (auto [id, go]: gameObjects) {
            go->populateCommandBuffer(commandBuffer, currentImage);
        }
    }

};
