#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec3 aTangent;

layout(set = 1, binding = 0) uniform ModelUniformBufferObject {
    mat4  model;
} mubo;

layout(set = 0, binding = 0) uniform CameraUniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 position;
} cubo;

layout(location = 0) out vec3 FragPos;      // Fragment position in world space
layout(location = 1) out vec3 Normal;       // Normal in world space
layout(location = 2) out vec2 TexCoords;    // Texture coordinates
layout(location = 3) out mat3 TBN;          // Tangent, Bitangent, Normal matrix

void main()
{
    vec4 worldPos =  mubo.model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    vec3 normal = normalize(mat3(mubo.model ) * aNormal);
    vec3 tangent = normalize(mat3(mubo.model ) * aTangent);
    vec3 bitangent = cross(normal, tangent);
    TBN = mat3(tangent, bitangent, normal);

    Normal = normal;
    TexCoords = aTexCoords;

    gl_Position = cubo.proj * cubo.view * worldPos;
}
