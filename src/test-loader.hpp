#include "App.hpp"
#include "animated-model/gltf-loader.hpp"
#include "animated-model/skin.hpp"

int main() {

    try {
        tinygltf::Model model = GltfLoader::loadGlTFFile("assets/models/CesiumMan/glTF/CesiumMan.gltf");
        std::vector<Skin *> skins{};
        for (const auto &skin: model.skins) {
            skins.push_back(new Skin(GltfLoader::loadSkin(model, skin)));
        }
        std::cout << "Skins: " << skins.size() << std::endl;

        for (auto skin: skins) {
            std::cout << "Skin: " << skin->getName() << std::endl;
            std::cout << "Vertices: " << skin->getVertices().size() << std::endl;

            std::cout << "Joints: " << std::endl;
            auto skinRoot = skin->getJoint(skin->getRootJointIndex());
            std::cout << "Root Joint: " << skinRoot->getName() << std::endl;
            for (auto i: skin->getJointIndices()) {
                auto joint = skin->getJoint(i);
                if (joint->parent != nullptr) {

                    std::cout << "Joint: " << joint->getName() << " Index: " << joint->getIndex() << " Parent Index: "
                              << joint->parent->getIndex() << " Parent Name: " << joint->parent->getName() << std::endl;
                } else {
                    std::cout << "Joint: " << joint->getName() << " Index: " << joint->getIndex() << " No Parent "
                              << std::endl;
                }
            }

        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
