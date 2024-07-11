// This has been adapted from the Vulkan tutorial
#pragma once

#include "modules/Starter.hpp"
#include "animated-model/skin.hpp"
#include "animated-model/gltf-loader.hpp"
#include "helper-structs.hpp"
#include "game-object.hpp"
#include "game-object-loader.hpp"

class App : public BaseProject {
protected:
    Skin *skin;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    Camera camera;

    GameObject *sphere = new GameObject();
    GameObject *ground = new GameObject();


    std::vector<GameObject *> gameObjects = {sphere, ground};
    std::vector<GameObject *> roads = {};

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
        camera.updateAspectRatio(Ar);
    }

    void loadRoad() {
        auto ROAD_COUNT = 10;
        auto between = 2.0f;
        auto result = GameObjectLoader::loadGltf("assets/models/low_road/scene.gltf");

        for (int i = 0; i < ROAD_COUNT; i++) {
            auto road = new GameObject();
            road->setName("Road " + i);
            road->setBaseTexture("assets/models/low_road/textures/Material.004_baseColor.jpeg",
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 true);
            road->setVertices(result.vertices);
            road->setIndices(result.indices);
            road->setWorldMatrix(result.Wm);
            road->scale(glm::vec3(1.0f, 1.0f, 10.0f));
            road->rotate(glm::vec3(-90.0f, 0.0f, 0.0f));

            road->move(glm::vec3(0.0f, i * between, 0.0f));
            roads.push_back(road);
            gameObjects.push_back(road);

        }
        for (int i = 0; i < ROAD_COUNT; i++) {
            auto road = new GameObject();
            road->setName("Road " + i + ROAD_COUNT);
//            road->setModel("assets/models/low_road/scene.gltf", ModelType::GLTF);

            road->setVertices(result.vertices);
            road->setIndices(result.indices);
            road->setWorldMatrix(result.Wm);
            road->setBaseTexture("assets/models/low_road/textures/Material.004_baseColor.jpeg",
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 true);

//            road->setCullMode(VK_CULL_MODE_NONE);
            road->scale(glm::vec3(1.0f, 1.0f, 1.0f));
            road->rotate(glm::vec3(-90.0f, 0.0f, 0.0f));

            road->move(glm::vec3(0.0f, -i * between, 0.0f));

            gameObjects.push_back(road);
            roads.push_back(road);

        }
    }

    void loadModels() {
        tinygltf::Model model = GltfLoader::loadGlTFFile("assets/models/pepsiman/scene.gltf");
        std::vector<Skin *> skins{};
        for (const auto &skin: model.skins) {
            skins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }

        skin = skins[0];

        GltfLoader::loadAnimations(skin, model);
        skin->setScaling(glm::vec3(0.5f));
        skin->updateWorldMatrix();

        sphere->setName("sphere");
        sphere->setModel("assets/models/sphere.obj", ModelType::OBJ);
        sphere->setBaseTexture("assets/textures/2k_sun.jpg", VK_FORMAT_R8G8B8A8_UNORM, true);
        sphere->scale(glm::vec3(0.1f));
        sphere->move(glm::vec3(-10.0f, 30.0f, 10.0f));

        loadRoad();

        ground->setModel("assets/models/SPW_Terrain_Grass_Flat.mgcg", MGCG);
        ground->setName("ground");
        ground->setBaseTexture("assets/textures/SPW_Natures_02.png", VK_FORMAT_R8G8B8A8_UNORM, true);
        ground->scale(glm::vec3(100.0f));
        ground->rotate(glm::vec3(90.0f, 0.0f, 0.0f));
        ground->move(glm::vec3(00.0f, 00.0f, .002f));
//        ground->setCullMode(VK_CULL_MODE_NONE);


        for (auto go: gameObjects) {
            go->updateWorld();
        }
    }

    glm::vec3 CameraInitialPosition = glm::vec3(0.0f, 0.75f, -2.0f);

    void setCamera() {
        camera.type = Camera::CameraType::lookat;
        camera.flipY = true;
        camera.setPosition(CameraInitialPosition);
        camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.setPerspective(90.0f, Ar, 0.1f, 256.0f);
        camera.setAspectRatio(Ar);

        camera.setRotationSpeed(10.0f);
        camera.setZoomSpeed(2.0f);
    }

    void localInit() {
        loadModels();
        setCamera();
        for (auto go: gameObjects) {
            DPSZs.uniformBlocksInPool += go->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += go->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += go->getPoolSizes().setsInPool;

            go->init(this, &camera);
        }

        auto skinDPSZs = Skin::getPoolSizes();
        skin->init(this, &camera, "assets/models/pepsiman/textures/Pepsiman_baseColor.png");
        DPSZs.uniformBlocksInPool += skinDPSZs.uniformBlocksInPool;
        DPSZs.texturesInPool += skinDPSZs.texturesInPool;
        DPSZs.setsInPool += skinDPSZs.setsInPool;


    }


    void pipelinesAndDescriptorSetsInit() {

        skin->createPipelineAndDescriptorSets();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsInit();
        }

    }

    void pipelinesAndDescriptorSetsCleanup() {

        skin->pipelinesAndDescriptorSetsCleanup();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsCleanup();
        }
    }


    void localCleanup() {
        skin->localCleanup();
        for (auto go: gameObjects) {
            go->localCleanup();
        }
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

        skin->bind(commandBuffer, currentImage);
        for (auto go: gameObjects) {
            go->populateCommandBuffer(commandBuffer, currentImage);
        }

    }

    bool pause = true;
    float HeroMovingSpeed = 0.1f;

    void updateUniformBuffer(uint32_t currentImage) {
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
            skin->setTranslation(glm::vec3(0));
            camera.setPosition(CameraInitialPosition);
        }
        if (glfwGetKey(window, GLFW_KEY_P)) {
            pause = false;
        }
        if (glfwGetKey(window, GLFW_KEY_O)) {
            pause = true;
        }

        if (!pause) {
            skin->move(glm::vec3(0.0f, -HeroMovingSpeed, 0.0f));
//            camera.setPosition(skin->getPosition() + glm::vec3(0.0f, 0.75f, 0.0f));
//            camera.lookAt(skin->getPosition(), glm::vec3(0.0f, 0.0f, -1.0f));
//            camera.setPosition(camera.position + glm::vec3(0.0f, 0.0f, HeroMovingSpeed/10));
//            camera.lookAt(skin->getPosition());
            camera.setPosition(CameraInitialPosition + glm::vec3(0.0f, 0.0f, -HeroMovingSpeed));
            skin->updateAnimation(frameTime);
        }
//        camera.lookAt(skin->getPosition(), glm::vec3(0.0f, 1.0f, 0.0f));
//        camera.rotate(glm::vec3(r.x * camera.rotationSpeed, -r.y * camera.rotationSpeed,
//                                r.z * camera.rotationSpeed));
//        camera.zoom(m.z * camera.zoomSpeed);
        skin->render(currentImage, axisInput);

        for (auto go: gameObjects) {
            go->render(currentImage);
        }

    }
};
