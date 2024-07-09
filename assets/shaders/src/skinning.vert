#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inJointWeights;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 0, binding = 1) uniform JointUBO {
    mat4 jointMatrices[100];
} jointTransformsUbo;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

void main() {
    mat4 skinMatrix =
    inJointWeights.x * jointTransformsUbo.jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointTransformsUbo.jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointTransformsUbo.jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointTransformsUbo.jointMatrices[int(inJointIndices.w)];

    vec4 skinnedPosition = skinMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model * skinnedPosition;

    fragNormal = mat3(ubo.model) * mat3(skinMatrix) * inNormal;
    fragUV = inUV;
}




