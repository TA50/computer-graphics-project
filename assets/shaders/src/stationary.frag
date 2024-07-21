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





void main(){

    vec3 Norm = normalize(fragNorm);

    vec3 lightDir =normalize(light.direction);
    vec3 EyeDir = normalize(fragPos - camera.position);
    vec3 lightColor = light.color.rgb;
    vec4 baseColor = texture(baseTex, fragUV);

    // BRDF
    // lambart diffuse:
    vec3 Diffuse = baseColor.rgb * max(dot(Norm, lightDir), 0.0);
    // specular: blinn specular
    vec3 H = normalize(lightDir + EyeDir);
    vec3 Specular = lightColor * pow(max(dot(Norm, H), 0.0), light.specularGamma);

    vec3 Ambient = ((Norm.x > 0 ? ambient.cxp : ambient.cxn) * (Norm.x * Norm.x) +
    (Norm.y > 0 ? ambient.cyp : ambient.cyn) * (Norm.y * Norm.y) +
    (Norm.z > 0 ? ambient.czp : ambient.czn) * (Norm.z * Norm.z)) * baseColor.rgb;
    // final color
    vec3 col  = (Diffuse + Specular) * lightColor + Ambient;
    // gamma correction
    col = pow(col, vec3(1.0/2.2));
    fragColor = vec4(col, 1);

}
