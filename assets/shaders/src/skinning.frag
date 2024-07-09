#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(set = 0, binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
//    vec3 normal = normalize(fragNormal);
//    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.75));
//    float diff = max(dot(normal, lightDir), 0.0);
//
//    vec4 texColor = texture(texSampler, fragUV);
//    outColor = diff * texColor;

    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
