// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-object.hpp"

class App : public BaseProject {
protected:
    Skin *skin;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    Camera camera;

    GameObject *sphere = new GameObject();
    GameObject *ground = new GameObject();
    GameObject *road = new GameObject();


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

    void loadModels() {
        tinygltf::Model model = GltfLoader::loadGlTFFile("assets/models/pepsiman/scene.gltf");
        std::vector<Skin *> skins{};
        for (const auto &skin: model.skins) {
            skins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }

        skin = skins[0];

        GltfLoader::loadAnimations(skin, model);
        sphere->setName("sphere");
        sphere->setModel("assets/models/sphere.obj", ModelType::OBJ);
        sphere->setBaseTexture("assets/textures/brown_mud.jpg", VK_FORMAT_R8G8B8A8_UNORM, true);

        road->setName("Road");
        road->setModel("assets/models/SPW_Medieval_Road_01.mgcg", ModelType::MGCG);
        road->setBaseTexture("assets/textures/SPW_Natures_02.png", VK_FORMAT_R8G8B8A8_UNORM, true);
        road->setCullMode(VK_CULL_MODE_NONE);
//        road->scale(glm::vec3(20.0f, 1.0f, 1.0f));
        road->rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        road->rotate(90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        road->move(glm::vec3(0.0f, 0.5f, -0.1f));

        ground->setModel("assets/models/SPW_Terrain_Grass_Flat.mgcg", MGCG);
        ground->setName("ground");
        ground->setBaseTexture("assets/textures/SPW_Natures_02.png", VK_FORMAT_R8G8B8A8_UNORM, true);
//        ground->rotate(30.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        ground->rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        ground->scale(glm::vec3(20.0f));
        ground->setCullMode(VK_CULL_MODE_NONE);
    }

    void setCamera() {
        camera.type = Camera::CameraType::lookat;
        camera.flipY = true;
        camera.setPosition(glm::vec3(0.0f, 0.75f, -2.0f));
        camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.setPerspective(90.0f, Ar, 0.1f, 256.0f);
        camera.setAspectRatio(Ar);
    }

    void localInit() {
        loadModels();
        setCamera();
        auto modelPoolSizes = sphere->getPoolSizes();
        auto groundPoolSizes = ground->getPoolSizes();
        auto skinDPSZs = Skin::getPoolSizes();
        DPSZs.uniformBlocksInPool =
                skinDPSZs.uniformBlocksInPool
                + modelPoolSizes.uniformBlocksInPool
                + groundPoolSizes.uniformBlocksInPool
                + road->getPoolSizes().uniformBlocksInPool
                ;
        DPSZs.texturesInPool =
                skinDPSZs.texturesInPool + modelPoolSizes.texturesInPool +
                groundPoolSizes.texturesInPool + road->getPoolSizes().texturesInPool;
        DPSZs.setsInPool = skinDPSZs.setsInPool + modelPoolSizes.setsInPool + groundPoolSizes.setsInPool + road->getPoolSizes().setsInPool;


        skin->init(this, &camera,
                   "assets/models/pepsiman/textures/Pepsiman_baseColor.png");
        sphere->init(this, &camera);
        ground->init(this, &camera);
        road->init(this, &camera);


    }


    void pipelinesAndDescriptorSetsInit() {

        skin->createPipelineAndDescriptorSets();
        sphere->pipelinesAndDescriptorSetsInit();
        ground->pipelinesAndDescriptorSetsInit();
        road->pipelinesAndDescriptorSetsInit();
        std::cout << "Pipelines and Descriptor Sets initialization completed!\n";
    }

    void pipelinesAndDescriptorSetsCleanup() {

        skin->pipelinesAndDescriptorSetsCleanup();

        sphere->pipelinesAndDescriptorSetsCleanup();
        ground->pipelinesAndDescriptorSetsCleanup();
        road->pipelinesAndDescriptorSetsCleanup();

    }


    void localCleanup() {
        skin->localCleanup();
        sphere->localCleanup();
        ground->localCleanup();
        road->localCleanup();
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

        skin->bind(commandBuffer, currentImage);
//        sphere->populateCommandBuffer(commandBuffer, currentImage);
        ground->populateCommandBuffer(commandBuffer, currentImage);
        road->populateCommandBuffer(commandBuffer, currentImage);


    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        getSixAxis(deltaT, m, r, fire);

        camera.rotate(glm::vec3(r.x * camera.rotationSpeed, -r.y * camera.rotationSpeed, 0.0f));
        camera.zoom(m.z * camera.zoomSpeed);

        AxisInput axisInput{};
        axisInput.axis = m;
        axisInput.rotation = r;
        axisInput.deltaTime = deltaT;
        skin->render(currentImage, axisInput, frameTime);
//        sphere->render(currentImage);
        ground->render(currentImage);
        road->render(currentImage);


    }
};
