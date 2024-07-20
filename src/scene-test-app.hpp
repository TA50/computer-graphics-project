// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-objects/game-object-loader.hpp"
#include "scene/only-pepsiman-scene.hpp"

class SceneTestApp : public BaseProject {
protected:

    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;
    OnlyPepsimanScene testScene = OnlyPepsimanScene("test-scene", "assets/pepsiman-alone.json");
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

    }

    void localInit() {
        testScene.load(this, Ar);
        PoolSizes p = testScene.getPoolSizes();
        DPSZs.uniformBlocksInPool += p.uniformBlocksInPool;
        DPSZs.texturesInPool += p.texturesInPool;
        DPSZs.setsInPool += p.setsInPool;

        testScene.init();


    }

    bool debounce = false;
    int curDebounce = -1;

    void updateUniformBuffer(uint32_t currentImage) override {

        checkKey();
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        UserInput userInput = {m, r, curDebounce, deltaT};

        testScene.updateUniformBuffer(currentImage, userInput);


    }


    void checkKey() {
        for (int i = 32; i <= 348; i++) {
            if ( glfwGetKey(window, i)) {
                if (!debounce ) {
                    debounce = true;
                    curDebounce = i;
                }
            } else {
                if ((curDebounce == i) && debounce) {
                    debounce = false;
                    curDebounce = -1;
                }
            }

        }
    }

    void pipelinesAndDescriptorSetsInit() override {
        testScene.pipelinesAndDescriptorSetsInit();
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        testScene.pipelinesAndDescriptorSetsCleanup();
    }


    void localCleanup() override {
        testScene.localCleanup();
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        testScene.populateCommandBuffer(commandBuffer, currentImage);
    }

};
