// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-object.hpp"
#include "game-object-loader.hpp"
#include "world-loader.hpp"


class App : public BaseProject {
protected:
    const std::string pepsimanId = "pepsiman";
    const std::string lunaId = "luna";
    Skin *pepsiman;
    Skin *luna;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    GameConfig gameConfig;


    WorldLoader worldLoader = WorldLoader("assets/world.json");

    Camera camera;

    std::unordered_map<std::string, GameObject *> gameObjects;
    std::unordered_map<std::string, Skin *> skins;

    glm::vec3 CameraInitialPosition = glm::vec3(0, 2.07f, 2);

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

    void loadModels() {
        gameObjects = worldLoader.loadGameObjects();
        std::cout << "Game Objects loaded" << std::endl;
        skins = worldLoader.loadSkins();
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

    void setGameConfig() {

        worldLoader.setGame(&gameConfig);
    }

    void setCamera() {
        worldLoader.setCamera(&camera, Ar);
        camera.updatePerspective();
    }


    void localInit() {

        worldLoader.readMatrixJson();
        loadModels();
        setCamera();
        setWorld();
        setGameConfig();


        for (auto [id, go]: gameObjects) {
            DPSZs.uniformBlocksInPool += go->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += go->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += go->getPoolSizes().setsInPool;

            go->init(this, &camera);
        }
        for (auto [id, sk]: skins) {
            DPSZs.uniformBlocksInPool += sk->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += sk->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += sk->getPoolSizes().setsInPool;

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
