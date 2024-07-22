#version 450

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoords;
layout(location = 3) in mat3 TBN;

layout(set = 1, binding = 1) uniform sampler2D baseColorMap;
layout(set = 1, binding = 2) uniform sampler2D metalnessRoughnessMap;
layout(set = 1, binding = 3) uniform sampler2D normalMap;
layout(set = 0, binding = 2) uniform AmbientUniformBufferObject{
    vec3 cxp;
    vec3 cxn;
    vec3 cyp;
    vec3 cyn;
    vec3 czp;
    vec3 czn;
} ambient;
layout(set = 0, binding = 1) uniform LightUniformBufferObject {
    vec3 position;
    vec3 direction;
    vec3 color;
    float specularGamma;
} light;

layout(set = 0, binding = 0) uniform CameraUniformBufferObject {
    mat4 view;
    mat4 projection;
    vec3 position;
    vec3 eyePos;
} camera;

layout(location = 0) out vec4 FragColor;

vec3 computeAmbient(vec3 normal) {
    vec3 ambientLight =
    abs(normal.x) * (normal.x > 0.0 ? ambient.cxp : ambient.cxn) +
    abs(normal.y) * (normal.y > 0.0 ? ambient.cyp : ambient.cyn) +
    abs(normal.z) * (normal.z > 0.0 ? ambient.czp : ambient.czn);

    return ambientLight;
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return num / denom;
}

// Geometry function (Smith method)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 reinhardToneMapping(vec3 hdrColor) {
    return hdrColor / (hdrColor + vec3(1.0));
}

void main() {
    // Sample textures
    vec3 albedo = texture(baseColorMap, TexCoords).rgb;
    vec3 mR = texture(metalnessRoughnessMap, TexCoords).rgb;
    float roughness = clamp(mR.g, 0.05, 1.0);// Ensure roughness is not too low
    float metalness = clamp(mR.b, 0.05, 1.0);// Ensure metalness is within valid range

    vec3 normal = texture(normalMap, TexCoords).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    // View direction
    vec3 V = normalize(camera.position - FragPos);

    // Use the normalized light direction for directional light
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001;// Prevent divide by zero
    vec3 specular = numerator / denominator;

    // Diffuse term (Lambertian)
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 diffuse = kD * albedo * 7 / 22;// albedo/pi

    // Final color
    vec3 lightColor = light.color;
    vec3 irradiance = lightColor * NdotL;
    // Ambient term

    vec3 ambientLight = computeAmbient(normal);
    vec3 amb = ambientLight * albedo;
    specular = pow(specular, vec3(3));
    vec3 finalColor = amb *0.0005+ (specular + diffuse*0.9995)  * irradiance;
    //    vec3 finalColor = amb *0.003+ (diffuse + specular) * irradiance ;

    // Apply gamma correction
    //    finalColor = finalColor * 2.0;
    finalColor = reinhardToneMapping(finalColor);
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    FragColor = vec4(finalColor, 1.0);
}
