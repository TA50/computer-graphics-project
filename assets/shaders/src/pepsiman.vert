#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in ivec4 inJointIndices;
layout(location = 4) in vec4 inJointWeights;
layout(location = 5) in vec3 inColor;
layout(location = 6) in vec4 inTan;



layout(set = 1, binding = 0) uniform ModelUniformBufferObject {
    mat4  model;
    mat4 jointTransformMatrices[100];
} ubo;

layout(set = 0, binding = 0) uniform CameraUniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    vec3 eyePos;
} cubo;


layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec4 fragTan;
layout (location = 3) out vec3 fragPos;

mat4 calcSkinMat() {
    mat4 skinMat =
    inJointWeights.x * ubo.jointTransformMatrices[int(inJointIndices.x)] +
    inJointWeights.y * ubo.jointTransformMatrices[int(inJointIndices.y)] +
    inJointWeights.z * ubo.jointTransformMatrices[int(inJointIndices.z)] +
    inJointWeights.w * ubo.jointTransformMatrices[int(inJointIndices.w)];
    return skinMat;
}

void main() {

    mat4 skinMat = calcSkinMat();
    mat4 viewModel = cubo.view * ubo.model;
    gl_Position = cubo.proj * viewModel * skinMat * vec4(inPosition.xyz, 1.0);

    outNormal = normalize(transpose(inverse(mat3(viewModel * skinMat))) * inNormal);
    fragTan = vec4((mat4(1) * vec4(inTan.xyz, 0.0)).xyz, inTan.w);
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    outUV = inUV;

}


