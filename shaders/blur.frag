#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
//uniform float blurRadius;

const vec3 size = vec3(256, 256, 30.);//width,height,radius
const int Quality = 8;
const int Directions = 16;
const float Pi = 6.28318530718;

void main(void) {
//    vec2 radius = 25./vec2(256, 256);//blurRadius;// /*size.z*/blurRadius/size.xy;
//    vec4 Color = texture2D(texture, texCoord);
//    for(float d = 0.0; d < Pi; d += Pi/float(Directions)) {
//        for(float i= 1.0/float(Quality); i <= 1.0; i += 1.0/float(Quality)) {
//            Color += texture2D(texture, texCoord + vec2(cos(d), sin(d))*radius*i);
//        }
//    }
//    Color /= float(Quality)*float(Directions) + 1.0;
//    gl_FragColor =  Color;
    gl_FragColor = mix(texture2D(texture, texCoord),
                       vec4(1.f, 0.f, 0.f, 1.f), 0.5f);
}
