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



vec4 calculatePosition(){

    // Calculate skinned matrix from weights and joint indices of the current vertex
    mat4 skinMat =
    inJointWeights.x * ubo.jointTransformMatrices[int(inJointIndices.x)] +
    inJointWeights.y * ubo.jointTransformMatrices[int(inJointIndices.y)] +
    inJointWeights.z * ubo.jointTransformMatrices[int(inJointIndices.z)] +
    inJointWeights.w * ubo.jointTransformMatrices[int(inJointIndices.w)];

    return  ubo.proj * ubo.view * ubo.model * skinMat * vec4(inPosition.xyz, 1.0);

}
void main() {

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

//    vec4 position = vec4(0.0);
//
//    for (int i = 0; i < 4; ++i) {
//        int jointIndex = int(inJointIndices[i]);   // Get the joint index
//        float weight = inJointWeights[i];          // Get the weight for this joint
//
//        mat4 jointTransform = ubo.jointTransformMatrices[jointIndex];  // Get the joint transformation matrix
//        position += weight * (jointTransform * vec4(inPosition, 1.0)); // Apply transformation and accumulate
//    }
//
//
//    gl_Position = ubo.proj * ubo.view * ubo.model * position;
//    gl_Position = ubo.proj * ubo.view  * inPosition;

    gl_Position = calculatePosition();
    fragUV = inUV;
    fragPos = inPosition;
    fragNorm = mat3(transpose(inverse(ubo.model))) * inNormal;
}




