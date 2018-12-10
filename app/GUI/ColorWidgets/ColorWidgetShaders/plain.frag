#version 330 core
in vec3 pos;

uniform vec4 RGBAColor;
uniform vec2 meshSize;

void main(void) {
    float posFrac = 0.5f*(1.f + pos.x);
    if(RGBAColor.a < 0.99999f) {
        vec3 meshColor;
        int hId = int(floor(posFrac/meshSize.x));
        int vId = int(floor(0.5f*(1.f + pos.y)/meshSize.y));
        if((hId + vId) % 2 == 0) {
            meshColor = vec3(0.2f, 0.2f, 0.2f);
        } else {
            meshColor = vec3(0.4f, 0.4f, 0.4f);
        }
        gl_FragColor = vec4(mix(meshColor, RGBAColor.rgb, RGBAColor.a), 1.f);
    }
    gl_FragColor = vec4(RGBAColor.rgb, 1.f);
}
