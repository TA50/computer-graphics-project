//#pragma once
//
//#include "modules/Starter.hpp"
//#include "modules/Scene.hpp"
//
//// The uniform buffer object used in this example
//struct UniformBufferObject {
//    alignas(16) glm::mat4 mvpMat;
//    alignas(16) glm::mat4 mMat;
//    alignas(16) glm::mat4 nMat;
//};
//
//struct GlobalUniformBufferObject {
//    alignas(16) glm::vec3 lightDir;
//    alignas(16) glm::vec4 lightColor;
//    alignas(16) glm::vec3 eyePos;
//    alignas(16) glm::vec4 eyeDir;
//};
//
//
//// The vertices data structures
//// Example
//struct Vertex {
//    glm::vec3 pos;
//    glm::vec2 UV;
//    glm::vec3 norm;
//};
//
//class Landscape {
//
//public:
//    Scene SC;
//    BaseProject *BP;
//    DescriptorSetLayout DSL;
//    VertexDescriptor VD;
//    Pipeline P;
//    DescriptorSet DS;
//
//    PoolSizes getPoolSizes() {
//        PoolSizes poolSizes{};
//        poolSizes.setsInPool = 1;
//        poolSizes.uniformBlocksInPool = 1;
//        poolSizes.texturesInPool = 1;
//        return poolSizes;
//    }
//
//    void init(BaseProject *bp) {
//
//        this->BP = bp;
//
//        // Descriptor Layouts [what will be passed to the shaders]
//        DSL.init(bp, {
//                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS},
//                {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
//                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_ALL_GRAPHICS}
//        });
//
//        // Vertex descriptors
//        VD.init(bp, {
//                {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
//        }, {
//                        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t) offsetof(Vertex, pos),
//                                sizeof(glm::vec3), POSITION},
//                        {0, 1, VK_FORMAT_R32G32_SFLOAT,    (uint32_t) offsetof(Vertex, UV),
//                                sizeof(glm::vec2), UV},
//                        {0, 2, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t) offsetof(Vertex, norm),
//                                sizeof(glm::vec3), NORMAL}
//                });
//
//        // Pipelines [Shader couples]
//        P.init(bp, &VD, "assets/shaders/bin/PhongVert.spv", "assets/shaders/bin/PhongFrag.spv", {&DSL});
//        P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
//                              VK_CULL_MODE_NONE, false);
//
//        SC.init(bp, &VD, DSL, P, "assets/models/scene.json");
//    }
//
//
//    void pipelinesAndDescriptorSetsInit() {
//        P.create();
//        SC.pipelinesAndDescriptorSetsInit(DSL);
//    }
//
//    void pipelinesAndDescriptorSetsCleanup() {
//        SC.pipelinesAndDescriptorSetsCleanup();
//        P.cleanup();
//    }
//
//
//    void localCleanup() {
//        P.destroy();
//        VD.cleanup();
//        DSL.cleanup();
//        SC.localCleanup();
//
//    }
//
//    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
//        P.bind(commandBuffer);
//        SC.populateCommandBuffer(commandBuffer, currentImage, P);
//    }
//
//    std::vector<std::string> trkScn = {"pln", "prm"};
//
//    void render(uint32_t currentImage) {
//        glm::vec3 CamPos = glm::vec3(SC.I[SC.InstanceIds["tb"]].Wm[3]);
//        static glm::vec3 dampedCamPos = CamPos;
//        auto CamTarget = glm::vec3(SC.I[SC.InstanceIds["tb"]].Wm[3]) + glm::vec3(0, 0, 1);
//        static float CamYaw = 0.0f;
//        static float CamPitch = 0.0f;
//        static float CamRoll = 0.0f;
//        static float CamSpeed = 0.1f;
//        float Ar = 4.0f / 3.0f;
//        auto M = MakeViewProjectionLookAt(dampedCamPos, CamTarget, glm::vec3(0, 1, 0), CamRoll, glm::radians(90.0f), Ar,
//                                          0.1f, 500.0f);
//        glm::mat4 ViewPrj = M;
//        UniformBufferObject ubo{};
//        glm::mat4 baseTr = glm::mat4(1.0f);
//
//        GlobalUniformBufferObject gubo{};
//        gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
//        gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//        gubo.eyePos = dampedCamPos;
//        gubo.eyeDir = glm::vec4(0);
//        gubo.eyeDir.w = 1.0;
//
//        for (std::vector<std::string>::iterator it = trkScn.begin(); it != trkScn.end(); it++) {
//            int i = SC.InstanceIds[it->c_str()];
//            ubo.mMat = SC.I[i].Wm * baseTr;
//            ubo.mvpMat = ViewPrj * ubo.mMat;
//            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
//
//            SC.DS[i]->map(currentImage, &ubo, sizeof(ubo), 0);
//            SC.DS[i]->map(currentImage, &gubo, sizeof(gubo), 2);
//        }
//
//
//    }
//
//    glm::mat4 MakeViewProjectionLookAt(glm::vec3 Pos, glm::vec3 Target, glm::vec3 Up, float Roll, float FOVy, float Ar,
//                                       float nearPlane, float farPlane) {
//        glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
//        Prj = glm::scale(Prj, glm::vec3(1.0f, -1.0f, 1.0f));
//
//        glm::mat4 View =
//                glm::rotate(glm::mat4(1.0f), -Roll, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::lookAt(Pos, Target, Up);
//        return Prj * View;
//    }
//};