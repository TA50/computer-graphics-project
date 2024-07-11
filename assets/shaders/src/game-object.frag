#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;


layout(set = 0, binding = 1) uniform sampler2D tex;



layout(location = 0) out vec4 fragColor;

void main(){
    //    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    //    float diff = max(dot(fragNorm, lightDir), 0.0);
    //    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    ////    fragColor = vec4(1.0, 0.0, 0.0, 1.0) ;
    fragColor =texture(tex, fragUV);

}
