#version 450
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 fragTan;
layout (location = 3) in vec3 fragPos;

layout(set = 1, binding = 1) uniform sampler2D samplerColorMap;
layout(set = 1, binding = 2) uniform sampler2D metalicTex;
layout(set = 1, binding = 3) uniform sampler2D normalTex;
//

layout(set = 0, binding = 1) uniform LightUniformBufferObject{
    vec3 position;
    vec3 direction;
    vec3 color;
    float specularGamma;
} light;

layout(set = 0, binding = 0) uniform CameraUniformBufferObject{
    mat4 view;
    mat4 projection;
    vec3 position;
    vec3 eyePos;
} camera;



layout(set = 0, binding = 2) uniform AmbientUniformBufferObject{
    vec3 cxp;
    vec3 cxn;
    vec3 cyp;
    vec3 cyn;
    vec3 czp;
    vec3 czn;
} ambient;


layout(location = 0) out vec4 outFragColor;


const vec3 Fdielectric = vec3(0.04);

float D(float alpha, vec3 N, vec3 H) {
    float numerator = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float denominator = 3.14159 * pow(NdotH * NdotH * (alpha * alpha - 1.0) + 1.0, 2.0);
    return numerator / max(denominator, 0.000001);
}

float G1(float alpha, vec3 N, vec3 X) {
    float numerator = max(dot(N, X), 0.0);
    float k = alpha / 2.0;
    float denominator = max(dot(N, X), 0.0) * (1.0 - k) + k;
    return numerator / max(denominator, 0.000001);
}

float G(float alpha, vec3 N, vec3 L, vec3 V) {
    return G1(alpha, N, L) * G1(alpha, N, V);
}

vec3 F(vec3 F0, vec3 V, vec3 H) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - max(dot(V, H), 0.0), 5.0);
}

void main() {
    vec3 baseColor = texture(samplerColorMap, inUV).rgb;
    vec3 EyeDir = normalize(camera.eyePos);

    vec3 lightDir = normalize(light.direction);
    vec3 lightColor = light.color.rgb;

    vec3 L = normalize(-light.direction);
    vec3 V = normalize(EyeDir - fragPos);
    vec3 H = normalize(L + V);
    vec3 Norm = normalize(inNormal);

    vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
    vec3 Bitan = cross(Norm, Tan) * fragTan.w;
    mat3 tbn = mat3(Tan, Bitan, Norm);
    vec4 nMap = texture(normalTex, inUV);
    vec3 N = tbn * (nMap.xyz * 2.0 - 1.0);
    vec4 metallicColor = texture(metalicTex, inUV);
    vec4 roughnessColor = texture(metalicTex, inUV);

    float metallicFactor = metallicColor.b;// metallic value in blue channel
    float roughnessFactor = roughnessColor.g;// roughness value in green channel

    vec3 F0 = mix(Fdielectric, baseColor, metallicFactor);
    vec3 Ks = F(F0, V, H);
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallicFactor);

    vec3 lambert = clamp((baseColor.rgb / 3.14159), 0.0, 1.0);

    float alpha = roughnessFactor * roughnessFactor;
    vec3 CTNumerator = D(alpha, N, H) * G(alpha, N, V, L) * F(F0, V, H);
    float CTDenominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
    vec3 CT = CTNumerator / max(CTDenominator, 0.000001);

    vec3 metalRoughColor = (Kd * lambert + CT) * lightColor * max(dot(N, L), 0.0);

    vec3 ambientDiffuse = lightColor * 0.8 * baseColor * Kd;
    vec3 ambientSpecular = lightColor * 0.2 * Ks;
    vec3 Ambient = baseColor * 0.0045f;


    vec4 color = vec4(metalRoughColor + Ambient, 1.0);
    color = pow(color, vec4(1.0 / 2.2));
    outFragColor = color;

}