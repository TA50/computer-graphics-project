#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;


layout(set = 0, binding = 1) uniform sampler2D baseTex;
layout(set = 0, binding = 2) uniform sampler2D metalicTex;
layout(set = 0, binding = 3) uniform sampler2D roughnessTex;
layout(set = 0, binding = 4) uniform sampler2D diffuseTex;

layout(set = 1, binding = 0) uniform BlinnUniformBufferObject {
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
} gubo;

layout(location = 0) out vec4 fragColor;

void main(){
    //    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    //    float diff = max(dot(fragNorm, lightDir), 0.0);
    //    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    ////    fragColor = vec4(1.0, 0.0, 0.0, 1.0) ;
    vec4 baseColor = texture(baseTex, fragUV);
    vec4 diffuse = texture(diffuseTex, fragUV);
    vec4 metallic = texture(metalicTex, fragUV);
    vec4 roughness = texture(roughnessTex, fragUV);

    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);

    vec3 lightDir = normalize(gubo.lightDir);
    vec3 lightColor = gubo.lightColor.rgb;

    vec3 Diffuse = texture(baseTex, fragUV).rgb * 0.975f * max(dot(Norm, lightDir),0.0);
    vec3 Specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)),0.0), 200.0));
    vec3 Ambient = texture(baseTex, fragUV).rgb * 0.025f;

    vec3 col  = (Diffuse + Specular) * lightColor + Ambient;

    fragColor = vec4(col, 1.0f);

//    fragColor =texture(baseTex, fragUV);
//
}
