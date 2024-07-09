#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in ivec4 inJointIndices;
layout(location = 4) in vec4 inJointWeights;
layout(location = 5) in vec3 inColor;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 jointTransformMatrices[100];
    vec4 lightPos;
} ubo;

layout(set = 0, binding = 1) uniform JointUBO {
    mat4 inverseBindMatrices[100];
} inverseBindMatricesUbo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;



mat4 calcSkinMat() {
    mat4 skinMat =
    inJointWeights.x * ubo.jointTransformMatrices[int(inJointIndices.x)] +
    inJointWeights.y * ubo.jointTransformMatrices[int(inJointIndices.y)] +
    inJointWeights.z * ubo.jointTransformMatrices[int(inJointIndices.z)] +
    inJointWeights.w * ubo.jointTransformMatrices[int(inJointIndices.w)];
    return skinMat;
}
void main() {
    outNormal = inNormal;
    outColor = inColor;
    outUV = inUV;

   mat4 skinMat = calcSkinMat();
    gl_Position = ubo.proj * viewModel * skinMat * vec4(inPosition.xyz, 1.0);

    outNormal = normalize(transpose(inverse(mat3(viewModel * skinMat))) * inNormal);

    vec4 pos = ubo.view * vec4(inPosition, 1.0);
    vec3 lPos = mat3(ubo.view) * ubo.lightPos.xyz;
    outLightVec = lPos - pos.xyz;
    outViewVec = -pos.xyz;
}




