#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;


layout(set = 1, binding = 1) uniform sampler2D baseTex;
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




layout(location = 0) out vec4 fragColor;


// Phong
vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 LD) {
    vec3 Diffuse;
    vec3 Specular;
    Diffuse = Albedo * max(dot(Norm, LD), 0.0f);
    Specular = vec3(pow(max(dot(EyeDir, -reflect(LD, Norm)), 0.0f), 160.0f));

    return Diffuse + Specular;
}
void main(){

    vec3 Norm = normalize(fragNorm);

    vec3 lightDir =normalize(light.direction);
    vec3 EyeDir = normalize(camera.eyePos - fragPos);
    vec3 lightColor = light.color.rgb;
    vec4 baseColor = texture(baseTex, fragUV);

    vec3 Ambient = ((Norm.x > 0 ? ambient.cxp : ambient.cxn) * (Norm.x * Norm.x) +
    (Norm.y > 0 ? ambient.cyp : ambient.cyn) * (Norm.y * Norm.y) +
    (Norm.z > 0 ? ambient.czp : ambient.czn) * (Norm.z * Norm.z)) * baseColor.rgb;


    vec3 RendEqSol = BRDF(baseColor.rgb, Norm, EyeDir, light.direction) * light.color;

    RendEqSol += Ambient;
    fragColor  = vec4(RendEqSol, 1.0);

}
