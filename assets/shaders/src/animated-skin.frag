#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in  vec4 tan;


layout(set = 1, binding = 1) uniform sampler2D baseTex;
layout(set = 1, binding = 2) uniform sampler2D normalMap;

layout(set = 0, binding = 1) uniform LightUniformBufferObject{
    vec3 position;
    vec3 direction;
    vec3 color;
    float specularGamma;
} lubo;



layout(set = 0, binding = 2) uniform AmbientUniformBufferObject{
    vec3 cxp;
    vec3 cxn;
    vec3 cyp;
    vec3 cyn;
    vec3 czp;
    vec3 czn;
} ambient;



layout(set = 0, binding = 0) uniform CameraUniformBufferObject{
    mat4 view;
    mat4 projection;
    vec3 position;
} cubo;


layout(location = 0) out vec4 fragColor;

void main(){

    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(cubo.position - fragPos);

    vec3 lightDir = normalize(lubo.direction);
    vec3 lightColor = lubo.color.rgb;

    vec3 Tan = normalize(tan.xyz - Norm * dot(tan.xyz, Norm));
    vec3 Bitan = cross(Norm, Tan) * tan.w;
    mat3 tbn = mat3(Tan, Bitan, Norm);
    vec4 nMap = texture(normalMap, fragUV);
    vec3 N = tbn * (nMap.xyz * 2.0 - 1.0);

    vec3 baseColor = texture(baseTex, fragUV).rgb;

    vec3 Diffuse = baseColor * max(dot(N, lightDir), 0.0);
    //    vec3 Specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)), 0.0), 200.0));
    vec3 R = -reflect(lightDir, N);
    float cl = clamp(dot(R, EyeDir), 0.0, 1.0);
    vec3 ms = lightColor;
    vec3 Specular = ms * vec3(pow(cl, 64.0));
    vec3 Ambient = texture(baseTex, fragUV).rgb * 0.025f;

    vec3 col  = lightColor * (Diffuse + Specular) + Ambient;
    col = pow(col, vec3(1/2.2));
    fragColor = vec4(col, 1);

}
