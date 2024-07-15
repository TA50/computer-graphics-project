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
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 ViewMatrix;
    float Ar;



    WorldLoader worldLoader = WorldLoader("assets/world.json");

    Camera camera;

    GameObject *sphere = new GameObject("sphere");
    GameObject *ground = new GameObject("ground");
    GameObject *terrian = new GameObject("terrian");
    GameObject *city = new GameObject("city");
    GameObject * tunnel = new GameObject("tunnel");

//    std::vector<GameObject *> gameObjects = {};
    std::vector<GameObject *> gameObjects = {tunnel};
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
        Ar = (float) w / (float)h;
        camera.aspect = Ar;
        camera.updatePerspective();
    }

    int ROAD_COUNT = 10;

    void loadRoad() {
        auto result = GameObjectLoader::loadGltf("assets/models/low_road/scene.gltf");

        for (int i = 0; i < ROAD_COUNT; i++) {
            auto road = new GameObject("road_" + i);
            road->setName("Road " + i);
            road->setBaseTexture("assets/models/low_road/textures/Material.004_baseColor.jpeg",
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 true);
            road->setCullMode(VK_CULL_MODE_NONE);
            road->setVertices(result.vertices);
            road->setIndices(result.indices);
            roads.push_back(road);
            gameObjects.push_back(road);

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

//        skin->setRotation(glm::vec3(0, 90, 90));
        sphere->setName("sphere");
        sphere->setModel("assets/models/sphere.obj", ModelType::OBJ);
        sphere->setBaseTexture("assets/textures/2k_sun.jpg", VK_FORMAT_R8G8B8A8_UNORM, true);
        loadRoad();

        ground->setModel("assets/models/SPW_Terrain_Grass_Flat.mgcg", MGCG);
        ground->setName("ground");
        ground->setBaseTexture("assets/textures/SPW_Natures_02.png", VK_FORMAT_R8G8B8A8_UNORM, true);
        ground->setCullMode(VK_CULL_MODE_NONE);


        terrian->setModel("assets/models/terrian/scene.gltf", GLTF);
        terrian->setName("terrian");
        terrian->setBaseTexture("assets/textures/terrian/GREEN_baseColor.png", VK_FORMAT_R8G8B8A8_UNORM, true);
        terrian->setCullMode(VK_CULL_MODE_NONE);
        auto result = GameObjectLoader::loadGltf("assets/models/City_6_Industrial.gltf");
        city->setVertices(result.vertices);
        city->setIndices(result.indices);
        city->setLocalMatrix(result.Wm);
        city->setName("city");
        city->setBaseTexture("assets/textures/textures.png", VK_FORMAT_R8G8B8A8_UNORM, true);
        city->setCullMode(VK_CULL_MODE_NONE);


        result = GameObjectLoader::loadGltf("assets/models/tunnel_new/untitled.gltf");
        tunnel->setVertices(result.vertices);
        tunnel->setIndices(result.indices);
        tunnel->setLocalMatrix(result.Wm);
        tunnel->setName("tunnel");
        tunnel->setBaseTexture("assets/models/tunnel_road/textures/tunnel_road_texture_baseColor.jpeg", VK_FORMAT_R8G8B8A8_UNORM, true);
        tunnel->setCullMode(VK_CULL_MODE_NONE);


    }

    void setWorld() {
        std::vector<GameObject *> transformed = {tunnel};
        for (auto go: transformed) {

            auto scale = worldLoader.get(go->getId(), SCALE);
            auto rotation = worldLoader.get(go->getId(), ROTATE);
            auto translation = worldLoader.get(go->getId(), TRANSLATE);
            go->move(translation);
            go->rotate(rotation);
            go->scale(scale);
            go->setWorldMatrix(glm::mat4(1));
            go->updateWorld();

            auto model = go->getModel();
            std::cout << go->getName();
            Printer::printArrArr(model);
        }

        float between = 2.0f;
        for (int i = 0; i < ROAD_COUNT; i++) {

            auto road = roads[i];
            auto scale = worldLoader.get("road_base", SCALE);
            auto rotation = worldLoader.get("road_base", ROTATE);
            auto translation = worldLoader.get("road_base", TRANSLATE);
            translation.y -= (float) i * between;
            scale.x += (scale.x / (float) ROAD_COUNT) * (float) (i + 1);
            road->move(translation);
            road->rotate(rotation);
            road->scale(scale);
            road->setWorldMatrix(glm::mat4(1));
            road->updateWorld();
        }

    }


    void setCamera() {
//        camera.setEuler(M_PI, glm::radians(20.0f), glm::radians(90.0f));
//        camera.TargetDelta = glm::vec3(2, 2, 2);
//        camera.setPosition(CameraInitialPosition);
//        camera.fov = glm::radians(45.0f);
//        camera.aspect = Ar;
//        camera.znear = 0.1f;
//        camera.zfar = 500.0f;
//
        worldLoader.setCamera(&camera, Ar);
        camera.updatePerspective();

    }

    void setHero() {
        worldLoader.setHero(HeroMovingSpeed, Radius);
    }

    void localInit() {

        worldLoader.readMatrixJson();
        loadModels();
        setCamera();
        setHero();
        setWorld();


        for (auto go: gameObjects) {
            DPSZs.uniformBlocksInPool += go->getPoolSizes().uniformBlocksInPool;
            DPSZs.texturesInPool += go->getPoolSizes().texturesInPool;
            DPSZs.setsInPool += go->getPoolSizes().setsInPool;

            go->init(this, &camera);
        }

        std::cout << "Game Objects initialized" << std::endl;
        auto skinDPSZs = Skin::getPoolSizes();
        skin->init(this, &camera, "assets/models/pepsiman/textures/Pepsiman_baseColor.png");
        std::cout << "Skin initialized" << std::endl;
        DPSZs.uniformBlocksInPool += skinDPSZs.uniformBlocksInPool;
        DPSZs.texturesInPool += skinDPSZs.texturesInPool;
        DPSZs.setsInPool += skinDPSZs.setsInPool;

        std::cout << "Uniform Blocks: " << DPSZs.uniformBlocksInPool << std::endl;
        std::cout << "Textures: " << DPSZs.texturesInPool << std::endl;
        std::cout << "Sets: " << DPSZs.setsInPool << std::endl;

    }


    bool pause = true;
    float HeroMovingSpeed = 2.0f;
    float tx = 0.0f;
    float ty = 0.0f;
    float Radius = 0.0f;
    float Theta = 0.0f;

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
            Theta = 0.0f;
            tx = 0.0f;
            ty = 0.0f;
//            setCamera();
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
            setHero();
            std::cout << "Reset World" << std::endl;
        }



        if (!pause) {
            // Theta = (Theta > 360 ? Theta = 0 : Theta);
            tx = Radius * sin(glm::radians(-Theta));
            ty = Radius * cos(glm::radians(-Theta));
            Theta += HeroMovingSpeed;
            if(Theta > 360) {
                Theta = 0;
            }
            skin->move(glm::vec3(-tx, -ty, 0));
            skin->updateAnimation(frameTime);
        }

        camera.rotate(-r.y * deltaT, -r.x * deltaT, -r.z * deltaT);
//        camera.translate(glm::vec3(-m.x * deltaT, -m.y * deltaT, -m.z * deltaT));
        if (glfwGetKey(window, GLFW_KEY_V)) {
            camera.print();
            auto pos = skin->getPosition();
            std::cout << "Skin Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
            std::cout << "Hero Speed: " << HeroMovingSpeed << std::endl;
            std::cout << "Hero Radius: " << Radius << std::endl;
            std::cout << "Hero tx: " << -tx << std::endl;
            std::cout << "Hero ty: " << -ty << std::endl;
            std::cout << "Hero Theta: " << Theta << std::endl;
        }

        camera.lookAt(skin->getPosition());
        camera.updateWorld();
        camera.updateViewMatrix();
//        camera.updatePerspective();
        skin->render(currentImage, axisInput);


        for (auto go: gameObjects) {
            go->render(currentImage);
        }

    }


    void pipelinesAndDescriptorSetsInit() override {

        skin->createPipelineAndDescriptorSets();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsInit();
        }

    }

    void pipelinesAndDescriptorSetsCleanup() override {

        skin->pipelinesAndDescriptorSetsCleanup();
        for (auto go: gameObjects) {
            go->pipelinesAndDescriptorSetsCleanup();
        }
    }


    void localCleanup() override {
        skin->localCleanup();
        for (auto go: gameObjects) {
            go->localCleanup();
        }
    }


    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {

        skin->bind(commandBuffer, currentImage);
        for (auto go: gameObjects) {
            go->populateCommandBuffer(commandBuffer, currentImage);
        }

    }

};
