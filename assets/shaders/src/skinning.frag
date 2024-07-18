#version 450
//
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;
layout (location = 5) in vec4 fragTan;
layout (location = 6) in vec3 fragPos;

layout(set = 0, binding = 2) uniform sampler2D samplerColorMap;
layout(set = 0, binding = 3) uniform sampler2D metalicTex;
layout(set = 0, binding = 4) uniform sampler2D roughnessTex;
layout(set = 0, binding = 5) uniform sampler2D normalTex;
//
layout(set = 1, binding = 0) uniform BlinnUniformBufferObject {
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    vec3 cameraPos;
} gubo;

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

//void main() {
////    vec4 color = vec4(1, 0,0,1);
//    vec4 color = texture(samplerColorMap, inUV);
//    vec3 N = normalize(inNormal);
//    vec3 L = normalize(inLightVec);
//    vec3 V = normalize(inViewVec);
//    vec3 R = reflect(-L, N);
//    vec3 diffuse = inColor;
//    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
//    outFragColor = color * 10;
////    outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
//}

void main() {
    vec3 baseColor = texture(samplerColorMap, inUV).rgb;
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);

    vec3 lightDir = normalize(gubo.lightDir);
    vec3 lightColor = gubo.lightColor.rgb;

    vec3 L = normalize(gubo.lightDir - fragPos);
    vec3 V = normalize(gubo.cameraPos.xyz - fragPos);
    vec3 H = normalize(L + V);
    vec3 Norm = normalize(inNormal);



    if(fragTan.x == 0.0 && fragTan.y == 0.0 && fragTan.z == 0.0) {
        vec3 R = reflect(-L, Norm);
        vec3 diffuse = max(dot(Norm, L), 0.0) * baseColor.rgb;
        vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * baseColor.rgb;
        outFragColor = vec4(diffuse + specular, 1.0);
        outFragColor = vec4(1,0,0, 1.0);
    }else {
        vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
        vec3 Bitan = cross(Norm, Tan) * fragTan.w;
        mat3 tbn = mat3(Tan, Bitan, Norm);
        vec4 nMap = texture(normalTex, inUV);
        vec3 N = normalize(tbn * (vec3(-1, 1, -1) + nMap.rgb * vec3(2, -2, 2)));
        vec4 metallicColor = texture(metalicTex, inUV);
        vec4 roughnessColor = texture(roughnessTex, inUV);

        float metallicFactor = metallicColor.b; // metallic value in blue channel
        float roughnessFactor = roughnessColor.g; // roughness value in green channel

//        vec3 F0 = vec3(0.77, 0.78, 0.78); // sRGB
//        vec3 F0 = mix(Fdielectric, baseColor.rgb, metallicColor.rgb);
//        vec3 F0 = vec3(0.56, 0.57, 0.58); // linear
//        vec3 Ks = F(F0, V, H);
//        vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallicFactor);
//
//        vec3 lambert = (baseColor.rgb / 3.14159);
//
//        float alpha = roughnessFactor * roughnessFactor;
//        vec3 CTNumerator = D(alpha, N, H) * G(alpha, N, V, L) * F(F0, V, H);
//        float CTDenominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
//        vec3 CT = CTNumerator / max(CTDenominator, 0.000001);
//
//        vec3 Ambient = baseColor.rgb * 0.025f;
//        vec3 metalRoughColor = (Kd * lambert + CT) * lightColor;
//
//
        vec3 Diffuse = texture(samplerColorMap, inUV).rgb * max(dot(N, L),0.0);
        vec3 Specular = vec3(pow(max(dot(N, normalize(L + EyeDir)),0.0), 64.0));
//        Ambient = texture(samplerColorMap, inUV).rgb * 0.025f;
//
        vec3 col  = (Diffuse + Specular) * lightColor;

        Diffuse = baseColor.rgb * max(dot(N, L),0.0f);
        Specular = vec3(pow(max(dot(V, -reflect(L, N)),0.0f), 16.0f));
        vec3 Specular2 = vec3(pow(max(dot(V, -reflect(L, N)),0.0f), 164.0f));
                vec3 Ambient = baseColor.rgb * 0.025f;

        col = (Diffuse *(1-metallicFactor*.5) + Specular * metallicFactor * baseColor.rgb) + Ambient + Specular2 * lightColor;


//        outFragColor = vec4(metallicFactor, 0,0, 1.0);
        outFragColor = vec4(col, 1.0);
    }
}


//layout (location = 0) in vec3 inNormal;
//layout (location = 1) in vec3 inColor;
//layout (location = 2) in vec2 inUV;
//layout (location = 3) in vec3 inViewVec;
//layout (location = 4) in vec3 inLightVec;
//
//layout(set = 0, binding = 2) uniform sampler2D samplerColorMap;
//
//layout(location = 0) out vec4 outFragColor;
//
//
//void main() {
//    vec4 color = texture(samplerColorMap, inUV) ;
//    vec3 N = normalize(inNormal);
//    vec3 L = normalize(inLightVec);
//    vec3 V = normalize(inViewVec);
//    vec3 R = reflect(-L, N);
//    vec3 diffuse = max(dot(N, L), 0.5) * inColor;
//    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
//    outFragColor =color;
////    outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
//}