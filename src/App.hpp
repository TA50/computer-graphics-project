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
    Skin *skin;
    Skin *villan;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;

    GameConfig gameConfig;


    WorldLoader worldLoader = WorldLoader("assets/world.json");

    Camera camera;

    GameObject *sphere = new GameObject("sphere");
    GameObject *road = new GameObject("road");
    GameObject *colaCan = new GameObject("cola-can");


    std::vector<GameObject *> gameObjects = {road, colaCan};
    std::vector<GameObject *> roads = {};

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
        tinygltf::Model model = GltfLoader::loadGlTFFile("assets/models/pepsiman/scene.gltf");
        std::vector<Skin *> skins{};
        for (const auto &skin: model.skins) {
            skins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }

        skin = skins[0];
        GltfLoader::loadAnimations(skin, model);

        // load golem
        model = GltfLoader::loadGlTFFile("assets/models/lynx_-_stars_-_animated/scene.gltf");

        std::vector<Skin *> gskins{};
        for (const auto &skin: model.skins) {
            gskins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }

        villan = gskins[0];
        GltfLoader::loadAnimations(villan, model, 2);

//        golem->setActiveAnimation(2);
        auto result = GameObjectLoader::loadGltf("assets/models/low_road/long road.gltf");
        road->setName("Road");
        road->setBaseTexture("assets/models/low_road/textures/Material.002_baseColor.jpeg",
                             VK_FORMAT_R8G8B8A8_UNORM,
                             true);
        road->setCullMode(VK_CULL_MODE_NONE);
        road->setVertices(result.vertices);
        road->setIndices(result.indices);

        result = GameObjectLoader::loadGltf("assets/models/cola-can/cola.gltf");
        colaCan->setName("Cola Can");
        colaCan->setBaseTexture("assets/models/cola-can/textures/Material_001_baseColor.png",
                                VK_FORMAT_R8G8B8A8_UNORM,
                                true);
        colaCan->setCullMode(VK_CULL_MODE_NONE);
        colaCan->setVertices(result.vertices);
        colaCan->setIndices(result.indices);


    }

    void setWorld() {
        std::vector<GameObject *> transformed = {road, colaCan};
        for (auto go: transformed) {

            auto scale = worldLoader.get(go->getId(), SCALE);
            auto rotation = worldLoader.get(go->getId(), ROTATE);
            auto translation = worldLoader.get(go->getId(), TRANSLATE);
            go->setTranslation(translation);
            go->setRotation(rotation);
            go->scale(scale);
//            go->setWorldMatrix(glm::mat4(1));
//            go->updateWorld();

//            auto model = go->getModel();
//            std::cout << go->getName();
//            Printer::printArrArr(model);

            go->changeModelCenter(glm::vec3(0, 0, 0));
        }


        villan->setTranslation(worldLoader.get("luna", TRANSLATE));
        villan->setRotation(worldLoader.get("luna", ROTATE));
        villan->setScaling(worldLoader.get("luna", SCALE));
        villan->updateWorldMatrix();
        villan->setCameraFollow(false);


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


        for (auto go: gameObjects) {
            DPSZs.uniformBlocksInPool += go->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += go->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += go->getPoolSizes().setsInPool;

            go->init(this, &camera);
        }

        std::cout << "Game Objects initialized" << std::endl;
        auto skinDPSZs = Skin::getPoolSizes();
        skin->init(this, &camera, "assets/models/pepsiman/textures/Pepsiman_baseColor.png");
        villan->init(this, &camera, "assets/models/lynx_-_stars_-_animated/textures/stars_baseColor.png");

        std::cout << "Skin initialized" << std::endl;
        DPSZs.uniformBlocksInPool += skinDPSZs.uniformBlocksInPool * 2 + skinDPSZs.uniformBlocksInPool * 2;
        DPSZs.texturesInPool += skinDPSZs.texturesInPool * 2;
        DPSZs.setsInPool += skinDPSZs.setsInPool * 2;


        std::cout << "Uniform Blocks: " << DPSZs.uniformBlocksInPool << std::endl;
        std::cout << "Textures: " << DPSZs.texturesInPool << std::endl;
        std::cout << "Sets: " << DPSZs.setsInPool << std::endl;

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
            skin->setTranslation(glm::vec3(0));
            villan->setTranslation(glm::vec3(0));
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
            skin->move(glm::vec3(0, -gameConfig.heroSpeed, 0));
            skin->updateAnimation(frameTime * gameConfig.heroAnimationSpeed);
            villan->move(glm::vec3(0, -gameConfig.villainSpeed, 0));
            villan->updateAnimation(frameTime*gameConfig.villainAnimationSpeed);

        }

        camera.rotate(-r.y * deltaT, -r.x * deltaT, -r.z * deltaT);
//        camera.translate(glm::vec3(-m.x * deltaT, -m.y * deltaT, -m.z * deltaT));
        if (glfwGetKey(window, GLFW_KEY_V)) {
            camera.print();
            auto pos = skin->getPosition();
            std::cout << "Skin Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
            gameConfig.print();
        }

        camera.lookAt(skin->getPosition());
        camera.updateWorld();
        camera.updateViewMatrix();
//        camera.updatePerspective();
        skin->render(currentImage, axisInput);
        villan->render(currentImage, axisInput);

        for (auto go: gameObjects) {
            go->render(currentImage);
        }

    }


    void pipelinesAndDescriptorSetsInit() override {

        skin->createPipelineAndDescriptorSets();
        villan->createPipelineAndDescriptorSets();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsInit();
        }

    }

    void pipelinesAndDescriptorSetsCleanup() override {

        skin->pipelinesAndDescriptorSetsCleanup();
        villan->pipelinesAndDescriptorSetsCleanup();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsCleanup();
        }
    }


    void localCleanup() override {
        skin->localCleanup();
        villan->localCleanup();
        for (auto go: gameObjects) {
            go->localCleanup();
        }
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {

        skin->bind(commandBuffer, currentImage);
        villan->bind(commandBuffer, currentImage);
        for (auto go: gameObjects) {
            go->populateCommandBuffer(commandBuffer, currentImage);
        }

    }

};
