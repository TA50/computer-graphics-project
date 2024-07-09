#pragma  once

#include "animated-model/node.hpp"
#include "animated-model/skin.hpp"


class GltfLoader {
public:
    static tinygltf::Model loadGlTFFile(const std::string &filename) {
        tinygltf::Model glTFInput;
        tinygltf::TinyGLTF gltfContext;
        std::string error, warning;
        bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);
        if (fileLoaded) {
            std::cout << "File loaded successfully" << std::endl;
            return glTFInput;
        } else {
            std::cerr << "Failed to load glTF file: " << filename << std::endl;
            std::cerr << error << std::endl;
            throw std::runtime_error("Failed to load glTF file");
        }
    }

    static Skin loadSkin(const tinygltf::Model &model, const tinygltf::Skin &gltfSkin) {
        int jointsCount = static_cast<int>(gltfSkin.joints.size());
        Skin skin = Skin(gltfSkin.name, jointsCount);

        // 1. Load Inverse Bind Matrices and joints
        auto inverseBindMatrices = std::vector<glm::mat4>(jointsCount);
        const tinygltf::Accessor &accessor = model.accessors[gltfSkin.inverseBindMatrices];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        std::unordered_map<int, Joint *> jointMap;

        const auto *data = reinterpret_cast<const float *>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t i = 0; i < jointsCount; ++i) {
            auto jointIndex = gltfSkin.joints[i];
            glm::mat4 inverseBindMatrix = glm::make_mat4(data + i * 16);
            auto node = model.nodes[jointIndex];

            auto *joint = new Joint(node.name, jointIndex);
            if (!node.translation.empty()) {
                joint->translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            }
            if (!node.rotation.empty()) {
                joint->rotation = glm::quat(static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]));
            }
            if (!node.scale.empty()) {
                joint->scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
            }

            if (node.matrix.size() == 16) {
                joint->setLocalMatrix(glm::make_mat4(node.matrix.data()));
            } else {
                joint->setLocalMatrix();
            }


            skin.addJoint(joint, inverseBindMatrix, jointIndex);
            jointMap[jointIndex] = joint;

        }


        // 3. Load Vertices and Indices (assuming single mesh and primitive)
        if(model.meshes.empty()){
            throw std::runtime_error("No meshes found in glTF file");
        }
        const tinygltf::Mesh &gltfMesh = model.meshes[0]; // assuming the first mesh
        const tinygltf::Primitive &primitive = gltfMesh.primitives[0]; // assuming the first primitive

        // Load vertices
        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::Accessor &normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::Accessor &jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
        const tinygltf::Accessor &weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];

        const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::BufferView &normalView = model.bufferViews[normalAccessor.bufferView];
        const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
        const tinygltf::BufferView &jointView = model.bufferViews[jointAccessor.bufferView];
        const tinygltf::BufferView &weightView = model.bufferViews[weightAccessor.bufferView];

        const auto *posData = reinterpret_cast<const float *>(&model.buffers[posView.buffer].data[
                posAccessor.byteOffset + posView.byteOffset]);
        const auto *normalData = reinterpret_cast<const float *>(&model.buffers[normalView.buffer].data[
                normalAccessor.byteOffset + normalView.byteOffset]);
        const auto *uvData = reinterpret_cast<const float *>(&model.buffers[uvView.buffer].data[uvAccessor.byteOffset +
                                                                                                uvView.byteOffset]);
        const auto *jointData = reinterpret_cast<const uint16_t *>(&model.buffers[jointView.buffer].data[
                jointAccessor.byteOffset + jointView.byteOffset]);
        const auto *weightData = reinterpret_cast<const float *>(&model.buffers[weightView.buffer].data[
                weightAccessor.byteOffset + weightView.byteOffset]);

        for (size_t i = 0; i < posAccessor.count; ++i) {
            SkinVertex vertex{};

            vertex.pos = glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
            vertex.normal = glm::vec3(normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2]);
            vertex.uv = glm::vec2(uvData[i * 2], uvData[i * 2 + 1]);
            vertex.jointIndices = glm::vec4(jointData[i * 4], jointData[i * 4 + 1], jointData[i * 4 + 2],
                                            jointData[i * 4 + 3]);
            vertex.jointWeights = glm::vec4(weightData[i * 4], weightData[i * 4 + 1], weightData[i * 4 + 2],
                                            weightData[i * 4 + 3]);

            skin.addVertex(vertex);
        }

        // Load indices
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexView.buffer];

        if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const auto *indices = reinterpret_cast<const uint16_t *>(&indexBuffer.data[indexAccessor.byteOffset +
                                                                                           indexView.byteOffset]);
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                skin.addIndex(indices[i]);
            }
        } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const auto *indices = reinterpret_cast<const uint32_t *>(&indexBuffer.data[indexAccessor.byteOffset +
                                                                                           indexView.byteOffset]);
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                skin.addIndex(indices[i]);
            }
        }


        // Set parent-child relationships
        for (size_t i = 0; i < jointsCount; ++i) {
            auto jointIndex = gltfSkin.joints[i];
            const auto &node = model.nodes[jointIndex];

            Joint *joint = jointMap[jointIndex];
            for (const auto &childIndex: node.children) {
                if (jointMap.find(childIndex) != jointMap.end()) {
                    Joint *childJoint = jointMap[childIndex];
                    joint->children.push_back(childJoint);
                    childJoint->parent = joint;
                }
            }
        }

        // Identify root joint
        if (gltfSkin.skeleton != -1) {
            int rootIndex = gltfSkin.skeleton;
            skin.setRootJoint(rootIndex);
        } else {
            // Optionally, determine root joint if not explicitly specified
            for (auto &jointPair: jointMap) {
                if (jointPair.second->parent == nullptr) {
                    skin.setRootJoint(jointPair.first);
                    break;
                }
            }
        }

        return skin;
    }

};