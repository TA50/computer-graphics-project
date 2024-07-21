#include "app.hpp"

int main() {
    App app;
    try {
        app.run();
//        auto model = GameObjectLoader::loadGltfMulti("assets/models/big_city/scene.gltf");
//        std::cout << model.meshes.size() << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
