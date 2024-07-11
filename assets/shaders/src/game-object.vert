#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec2 inColor;



layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragUV;


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4  model;
    mat4 view;
    mat4 modelView;
    mat4 mvp;
} ubo;

void main(){
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);


    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNorm = mat3(transpose(inverse(ubo.model))) * inNorm;
    fragUV = inUV;
}