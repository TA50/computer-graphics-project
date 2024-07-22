// This has been adapted from the Vulkan tutorial
#pragma once

#include <thread>
#include "modules/Starter.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-objects/game-object-loader.hpp"
#include "scene/only-pepsiman-scene.hpp"
#include "scene/test-scene.hpp"
#include "scene/scene-base.hpp"
#include "scene/city-scene.hpp"
#include "scene/road-scene.hpp"

class App : public BaseProject {
protected:

    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;
    OnlyPepsimanScene *onlyPepsimanScene = new OnlyPepsimanScene("pepsiman-only", "assets/pepsiman-alone.json");
    TestScene *testScene = new TestScene("test-scene", "assets/test_scene.json");
    CityScene *cityScene = new CityScene("test-scene", "assets/city_scene.json");
    RoadScene *roadScene = new RoadScene("road-scene", "assets/road_scene.json");

    glm::vec3 CameraInitialPosition = glm::vec3(0, 2.07f, 2);


    std::unordered_map<int, SceneBase *> scenes = {
            {GLFW_KEY_1, onlyPepsimanScene},
            {GLFW_KEY_2, roadScene},
            {GLFW_KEY_3, cityScene},
    };

    // Here you set the main application parameters
    void setWindowParameters() override {
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "App Name";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.01f, 0.01f, 0.01f, 1.0f};

        Ar = (float) windowWidth / (float) windowHeight;

    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) override {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float) w / (float) h;

    }

    void localInit() {

        for (auto [K, s]: scenes) {
            s->load(this, Ar);
            PoolSizes p = s->getPoolSizes();
            DPSZs.uniformBlocksInPool += p.uniformBlocksInPool;
            DPSZs.texturesInPool += p.texturesInPool;
            DPSZs.setsInPool += p.setsInPool;
            s->init();
        }
    }

    bool debounce = false;
    int curDebounce = -1;
    int curScene = GLFW_KEY_1;

    void updateUniformBuffer(uint32_t currentImage) override {


        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        UserInput userInput = {m, r, curDebounce, deltaT, Ar};

        for (auto [K, s]: scenes) {
            if (K == curDebounce && curScene != K && debounce) {
                curScene = curDebounce;
                std::cout << "Switching to scene: " << curScene << std::endl;
                RebuildPipeline();
                delay(100);
                curDebounce = -1;
                debounce = false;
                return;
            }
        }
        auto currentScene = scenes[curScene];
        currentScene->updateUniformBuffer(currentImage, userInput);

        checkKey();
    }


    void checkKey() {
        for (int i = 32; i <= 348; i++) {
            if (glfwGetKey(window, i)) {
                if (!debounce) {
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

    static void delay(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void pipelinesAndDescriptorSetsInit() override {
        for (auto [K, s]: scenes) {
            s->pipelinesAndDescriptorSetsInit();
        }
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        for (auto [K, s]: scenes) {
            s->pipelinesAndDescriptorSetsCleanup();
        }
    }


    void localCleanup() override {
        for (auto [K, s]: scenes) {
            s->localCleanup();
        }
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {

        for (auto [K, s]: scenes) {
            s->populateCommandBuffer(commandBuffer, currentImage);
        }
    }

};
