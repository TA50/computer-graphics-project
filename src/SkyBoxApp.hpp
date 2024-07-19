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

class SkyBoxApp : public BaseProject {
protected:

    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;




    Camera camera;
    SkyBox skybox;

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

    void localInit() {
        skybox.setModel("assets/models/SkyBoxCube.obj", OBJ);
        skybox.init(this, &camera);
        camera.lookAt(CameraInitialPosition);
        DPSZs.uniformBlocksInPool += skybox.getPoolSizes().uniformBlocksInPool;
        DPSZs.texturesInPool += skybox.getPoolSizes().texturesInPool;
        DPSZs.setsInPool += skybox.getPoolSizes().setsInPool;

    }


    void updateUniformBuffer(uint32_t currentImage) override {

        skybox.render(currentImage);

    }


    void pipelinesAndDescriptorSetsInit() override {
        skybox.pipelinesAndDescriptorSetsInit();
    }

    void pipelinesAndDescriptorSetsCleanup() override {
        skybox.pipelinesAndDescriptorSetsCleanup();
    }


    void localCleanup() override {
        skybox.localCleanup();
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
        skybox.populateCommandBuffer(commandBuffer, currentImage);
    }

};
