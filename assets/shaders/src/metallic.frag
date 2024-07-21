#version 450


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;
layout(location = 4) in vec3 fragBiTan;


layout(set = 1, binding = 1) uniform sampler2D baseTex;
layout(set = 1, binding = 2) uniform sampler2D mettalicTex;
layout(set = 1, binding = 3) uniform sampler2D normalTex;


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

    // Construct the TBN matrix
    mat3 TBN = mat3(fragTan, fragBiTan, fragNorm);
    vec3 norm = texture(normalTex, fragUV).rgb;
    norm = norm * 2.0 - 1.0;
    // Transform the normal from tangent space to world space
    vec3 worldNormal = normalize(TBN * norm);

    vec3 lightDir =normalize(light.direction);
    vec3 EyeDir = normalize(fragPos - camera.position);
    vec3 lightColor = light.color.rgb;
    vec4 baseColor = texture(baseTex, fragUV);

    // BRDF
    // lambart diffuse:
    vec3 Diffuse = baseColor.rgb * max(dot(norm, lightDir), 0.0);
    // specular: blinn specular
    vec3 H = normalize(lightDir + EyeDir);
    vec3 Ambient = ((norm.x > 0 ? ambient.cxp : ambient.cxn) * (norm.x * norm.x) +
    (norm.y > 0 ? ambient.cyp : ambient.cyn) * norm.y * norm.y) +
    (norm.z > 0 ? ambient.czp : ambient.czn) * (norm.z * norm.z) * baseColor.rgb;

    vec3 R = -reflect(lightDir, norm);
    vec3 Specular = lightColor * pow(clamp(dot(EyeDir, R), 0.0, 1.0), light.specularGamma);

    // final color
    vec3 col  = (Diffuse + Specular) * 0.975 * lightColor + Ambient * 0.025;
    // gamma correction
    col = pow(col, vec3(1.0/2.2));
    fragColor = vec4(col, 1);

}
