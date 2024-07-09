// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"


class App : public BaseProject {
protected:
    Skin *skin;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    Camera camera;

    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "App Name";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};

        Ar = (float) windowWidth / (float) windowHeight;


    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        std::cout << "Window resized to: " << w << " x " << h << "\n";
        Ar = (float) w / (float) h;
    }


    void localInit() {
        tinygltf::Model model = GltfLoader::loadGlTFFile("assets/models/CesiumMan/glTF/CesiumMan.gltf");
        std::vector<Skin *> skins{};
        for (const auto &skin: model.skins) {
            skins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }

        skin = skins[0];
        camera.type  = Camera::CameraType::lookat;
        camera.flipY = true;
        camera.setPosition(glm::vec3(0.0f, 0.75f, -2.0f));
        camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.setPerspective(60.0f, Ar, 0.1f, 256.0f);




        auto skinDPSZs = Skin::getPoolSizes();
        DPSZs.uniformBlocksInPool = skinDPSZs.uniformBlocksInPool; // 2 more uniform blocks
        DPSZs.texturesInPool =  skinDPSZs.texturesInPool + 1;       // 5 textures
        DPSZs.setsInPool =  skinDPSZs.setsInPool;       // 1 more descriptor sets

        skin->init(this, &camera);

        std::cout << "Initialization completed!\n";
        std::cout << "Uniform Blocks in the Pool  : " << DPSZs.uniformBlocksInPool << "\n";
        std::cout << "Textures in the Pool        : " << DPSZs.texturesInPool << "\n";
        std::cout << "Descriptor Sets in the Pool : " << DPSZs.setsInPool << "\n";


    }


    void pipelinesAndDescriptorSetsInit() {

        skin->createPipelineAndDescriptorSets();

        std::cout << "Pipelines and Descriptor Sets initialization completed!\n";
    }

    void pipelinesAndDescriptorSetsCleanup() {

        skin->pipelinesAndDescriptorSetsCleanup();


    }


    void localCleanup() {

        skin->localCleanup();

    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

        skin->bind(commandBuffer, currentImage);

        std::cout << "Command buffer populated!\n";
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {

        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        skin->updateUniformBuffers(currentImage);


    }
};
