#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;


layout(set = 1, binding = 1) uniform sampler2D baseTex;

layout(set = 0, binding = 1) uniform LightUniformBufferObject{
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
} lubo;

layout(location = 0) out vec4 fragColor;

void main(){

    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(lubo.eyePos - fragPos);

    vec3 lightDir = normalize(lubo.lightDir);
    vec3 lightColor = lubo.lightColor.rgb;

    vec3 Diffuse = texture(baseTex, fragUV).rgb * 0.975f * max(dot(Norm, lightDir), 0.0);
    vec3 Specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)), 0.0), 200.0));
    vec3 Ambient = texture(baseTex, fragUV).rgb * 0.025f;

    vec3 col  = (Diffuse + Specular) * lightColor + Ambient;

    fragColor = vec4(col, 1);

}
