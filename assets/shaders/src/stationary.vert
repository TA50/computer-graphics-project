#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;



layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;


layout(set = 1, binding = 0) uniform ModelUniformBufferObject {
    mat4  model;
} mubo;

layout(set = 0, binding = 0) uniform CameraUniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 position;
    vec3 eyePos;
} cubo;

void main(){
    mat4 mvp = cubo.proj * cubo.view * mubo.model;
    gl_Position = mvp* vec4(inPosition, 1.0);
    fragPos = vec3(mubo.model * vec4(inPosition, 1.0));
    fragNorm = mat3(transpose(inverse(mubo.model))) * inNorm;
    fragUV = inUV;
}