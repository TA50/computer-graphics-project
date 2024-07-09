#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in ivec4 inJointIndices;
layout(location = 4) in vec4 inJointWeights;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 jointTransformMatrices[100];
} ubo;

layout(set = 0, binding = 1) uniform JointUBO {
    mat4 inverseBindMatrices[100];
} inverseBindMatricesUbo;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;

void main() {
//    mat4 skinMatrix =
//    inJointWeights.x * jointTransformsUbo.jointMatrices[int(inJointIndices.x)] +
//    inJointWeights.y * jointTransformsUbo.jointMatrices[int(inJointIndices.y)] +
//    inJointWeights.z * jointTransformsUbo.jointMatrices[int(inJointIndices.z)] +
//    inJointWeights.w * jointTransformsUbo.jointMatrices[int(inJointIndices.w)];


    vec4 jointXCoord =  inverseBindMatricesUbo.inverseBindMatrices[inJointIndices.x] * ubo.jointTransformMatrices[inJointIndices.x]   * vec4(inPosition, 1.0f);
    vec4 jointYCoord =inverseBindMatricesUbo.inverseBindMatrices[inJointIndices.y] * ubo.jointTransformMatrices[inJointIndices.y] *   vec4(inPosition, 1.0f);
    vec4 jointZCoord = inverseBindMatricesUbo.inverseBindMatrices[inJointIndices.z] * ubo.jointTransformMatrices[inJointIndices.z] *  vec4(inPosition, 1.0f);
    vec4 jointWfCoord = inverseBindMatricesUbo.inverseBindMatrices[inJointIndices.w] * ubo.jointTransformMatrices[inJointIndices.w] *  vec4(inPosition, 1.0f);

    vec4 skinnedPosition = inJointWeights.x * jointXCoord + inJointWeights.y * jointYCoord + inJointWeights.z * jointZCoord + inJointWeights.w * jointWfCoord;
//
//
//
//    for (int i = 0; i < 4; ++i) {
//        float weight = inJointWeights[i];
//        if (weight > 0.0f) {
//            int jointIndex = inJointIndices[i];
//            mat4 inverseBindMatrix = inverseBindMatricesUbo.inverseBindMatrices[jointIndex];
//            mat4 jointTransformMatrix = ubo.jointTransformMatrices[jointIndex];
//
//            // Transform the vertex to joint local space
//            vec4 localPosition = inverseBindMatrix * vec4(inPosition, 1.0f);
//
//            // Transform the vertex from joint local space to world space
//            vec4 transformedPosition = jointTransformMatrix * localPosition;
//
//            // Accumulate the weighted transformed position
//            finalPosition += weight * transformedPosition;
//        }
//    }
//
////    vec4 skinnedPosition = skinMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model * skinnedPosition;
//    gl_Position = ubo.proj * ubo.view  * inPosition;

    fragUV = inUV;
    fragPos = vec3(ubo.model * skinnedPosition);
    fragNorm = mat3(transpose(inverse(ubo.model))) * inNormal;
}




