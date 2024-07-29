#pragma  once

#include <utility>

#include "animated-model/skin-data.hpp"


struct LoadedSkinVertexResult {
    std::vector<SkinVertex> vertices;
    std::vector<uint32_t> indices;
};

class GltfLoader {
public:
    static tinygltf::Model loadGlTFFile(const std::string &filename) {
        tinygltf::Model glTFInput;
        tinygltf::TinyGLTF gltfContext;
        std::string error, warning;
        bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);
        if (fileLoaded) {
            std::cout << filename << " : " << "File loaded successfully" << std::endl;
            return glTFInput;
        } else {
            std::cerr << filename << " : " << "Failed to load glTF file: " << filename << std::endl;
            std::cerr << error << std::endl;
            throw std::runtime_error("Failed to load glTF file");
        }
    }


    static Joint *
    loadJoint(const tinygltf::Model &model, int nodeIndex, Joint *parent, std::unordered_map<int, Joint *> *jointMap) {
        auto node = model.nodes[nodeIndex];
        auto *joint = new Joint(node.name, nodeIndex);

        joint->parent = parent;

        // Get the local node matrix
        // It's either made up from translation, rotation, scale or a 4x4 matrix
        if (node.translation.size() == 3) {
            joint->translation = glm::make_vec3(node.translation.data());
        }
        if (node.rotation.size() == 4) {
            glm::quat q = glm::make_quat(node.rotation.data());
            joint->rotation = glm::mat4(q);
        }
        if (node.scale.size() == 3) {
            joint->scale = glm::make_vec3(node.scale.data());
        }
        if (node.matrix.size() == 16) {
            joint->setLocalMatrix(glm::make_mat4(node.matrix.data()));
        } else {
            joint->setLocalMatrix(glm::mat4(1));
        }

        // Load children
        for (auto childIndex: node.children) {
            auto child = loadJoint(model, childIndex, joint, jointMap);
            joint->children.push_back(child);
        }

        jointMap->insert({nodeIndex, joint});
        return joint;
    }


    static LoadedSkinVertexResult loadVertices(const tinygltf::Model &model, const tinygltf::Skin &gltfSkin) {
        LoadedSkinVertexResult result;
        result.vertices = std::vector<SkinVertex>();
        result.indices = std::vector<uint32_t>();
        if (model.meshes.empty()) {
            throw std::runtime_error("No meshes found in glTF file");
        }
        const tinygltf::Mesh &gltfMesh = model.meshes[0]; // assuming the first mesh
        const tinygltf::Primitive &primitive = gltfMesh.primitives[0]; // assuming the first primitive

        // Load vertices
        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::Accessor &normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        const float *tangentData = nullptr;
        auto tIt = primitive.attributes.find("TANGENT");
        if (tIt != primitive.attributes.end()) {
            const tinygltf::Accessor tangentAccessor = model.accessors[primitive.attributes.find("TANGENT")->second];
            const tinygltf::BufferView &tangentView = model.bufferViews[tangentAccessor.bufferView];
            tangentData = reinterpret_cast<const float *>(&model.buffers[tangentView.buffer].data[
                tangentAccessor.byteOffset + tangentView.byteOffset]);
        }


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
            if (tangentData != nullptr) {
                vertex.tangent = glm::vec4(tangentData[i * 4], tangentData[i * 4 + 1], tangentData[i * 4 + 2],
                                           tangentData[i * 4 + 3]);
            } else {
                vertex.tangent = glm::vec4(0.0f);
            }
            auto jointIndices = glm::ivec4(jointData[i * 4], jointData[i * 4 + 1], jointData[i * 4 + 2],
                                           jointData[i * 4 + 3]);
            vertex.jointIndices = glm::ivec4(gltfSkin.joints[jointIndices[0]], gltfSkin.joints[jointIndices[1]],
                                             gltfSkin.joints[jointIndices[2]], gltfSkin.joints[jointIndices[3]]);

            vertex.jointWeights = glm::vec4(weightData[i * 4], weightData[i * 4 + 1], weightData[i * 4 + 2],
                                            weightData[i * 4 + 3]);

            vertex.inColor = glm::vec4(1.0f);
            result.vertices.push_back(vertex);
        }

        // Load indices
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexView.buffer];

        if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const auto *indices = reinterpret_cast<const uint16_t *>(&indexBuffer.data[indexAccessor.byteOffset +
                indexView.byteOffset]);
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                result.indices.push_back(indices[i]);
            }
        } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const auto *indices = reinterpret_cast<const uint32_t *>(&indexBuffer.data[indexAccessor.byteOffset +
                indexView.byteOffset]);
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                result.indices.push_back(indices[i]);
            }
        }
        return result;
    }


    // Skin Data (New)
    static SkinData loadSkinData(tinygltf::Model &model, const tinygltf::Skin &gltfSkin) {
        SkinData skin;
        // Load All Joints:
        std::unordered_map<int, Joint *> jointMap;

        auto scene = model.scenes[0];

        for (auto nodeIndex: scene.nodes) {
            auto *joint = loadJoint(model, nodeIndex, nullptr, &jointMap);
        }


        int jointsCount = static_cast<int>(gltfSkin.joints.size());


        // 1. Load Inverse Bind Matrices and joints
        auto inverseBindMatrices = std::vector<glm::mat4>(jointsCount);
        const tinygltf::Accessor &accessor = model.accessors[gltfSkin.inverseBindMatrices];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        //        std::unordered_map<int, Joint *> jointMap;

        // inverse bind matrices
        const auto *data = reinterpret_cast<const float *>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
        for (size_t i = 0; i < jointsCount; ++i) {
            auto jointIndex = gltfSkin.joints[i];
            glm::mat4 inverseBindMatrix = glm::make_mat4(data + i * 16);
            skin.addJoint(jointMap[jointIndex], inverseBindMatrix, jointIndex);
        }

        // find skin root
        int rootIndex = -1;
        for (auto [idx, joint]: jointMap) {
            if (model.nodes[idx].skin > -1) {
                rootIndex = idx;
                break;
            }
        }
        if (rootIndex == -1) {
            throw std::runtime_error("No root joint found for skin");
        }
        skin.setRootJoint(jointMap[rootIndex], rootIndex);


        // 3. Load Vertices and Indices (assuming single mesh and primitive)
        auto result = loadVertices(model, gltfSkin);

        skin.vertices = result.vertices;
        skin.indices = result.indices;


        // 4. Load animations
        skin.animations = loadSkinAnimations(model, jointMap);
        return skin;
    }


    static std::vector<Animation>
    loadSkinAnimations(tinygltf::Model &input, std::unordered_map<int, Joint *> jointMap) {
        std::vector<Animation> animations;
        animations.resize(input.animations.size());

        for (size_t i = 0; i < input.animations.size(); i++) {
            tinygltf::Animation glTFAnimation = input.animations[i];
            animations[i].name = glTFAnimation.name;

            // Samplers
            animations[i].samplers.resize(glTFAnimation.samplers.size());
            for (size_t j = 0; j < glTFAnimation.samplers.size(); j++) {
                tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
                AnimationSampler &dstSampler = animations[i].samplers[j];
                dstSampler.interpolation = glTFSampler.interpolation;

                // Read sampler keyframe input time values
                {
                    const tinygltf::Accessor &accessor = input.accessors[glTFSampler.input];
                    const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];
                    const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    const float *buf = static_cast<const float *>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        dstSampler.inputs.push_back(buf[index]);
                    }
                    // Adjust animation's start and end times
                    for (auto input: animations[i].samplers[j].inputs) {
                        if (input < animations[i].start) {
                            animations[i].start = input;
                        };
                        if (input > animations[i].end) {
                            animations[i].end = input;
                        }
                    }
                }

                // Read sampler keyframe output translate/rotate/scale values
                {
                    const tinygltf::Accessor &accessor = input.accessors[glTFSampler.output];
                    const tinygltf::BufferView &bufferView = input.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer &buffer = input.buffers[bufferView.buffer];
                    const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    switch (accessor.type) {
                        case TINYGLTF_TYPE_VEC3: {
                            const glm::vec3 *buf = static_cast<const glm::vec3 *>(dataPtr);
                            for (size_t index = 0; index < accessor.count; index++) {
                                alignas(16) auto b = buf[index];
                                auto v = glm::vec4(0.0f);
                                v.x = b[0];
                                v.y = b[1];
                                v.z = b[2];

                                dstSampler.outputsVec4.push_back(v);
                            }
                            break;
                        }
                        case TINYGLTF_TYPE_VEC4: {
                            alignas(16) const auto *buf = static_cast<const glm::vec4 *>(dataPtr);
                            //                            dstSampler.outputsVec4.resize(accessor.count);
                            for (size_t index = 0; index < accessor.count; index++) {
                                dstSampler.outputsVec4.push_back(buf[index]);
                            }
                            break;
                        }
                        default: {
                            std::cout << "unknown type" << std::endl;
                            break;
                        }
                    }
                }
            }


            // Channels
            animations[i].channels.resize(glTFAnimation.channels.size());
            for (size_t j = 0; j < glTFAnimation.channels.size(); j++) {
                tinygltf::AnimationChannel glTFChannel = glTFAnimation.channels[j];
                AnimationChannel &dstChannel = animations[i].channels[j];
                dstChannel.path = glTFChannel.target_path;
                dstChannel.samplerIndex = glTFChannel.sampler;
                dstChannel.joint = jointMap[glTFChannel.target_node];
            }
        }

        return animations;
    }
};
