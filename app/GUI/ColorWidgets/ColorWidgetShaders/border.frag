#version 330 core
in vec3 pos;

uniform vec2 borderSize;
uniform vec4 borderColor;

void main(void) {
    float posXFrac = 0.5f*(1.f + pos.x);
    float posYFrac = 0.5f*(1.f + pos.y);

    bool noBorderH = posXFrac > borderSize.x && posXFrac < 1.f - borderSize.x;
    bool noBorderV = posYFrac > borderSize.y && posYFrac < 1.f - borderSize.y;

    if(noBorderH && noBorderV) {
        gl_FragColor = vec4(0.f, 0.f, 0.f, 0.f);
        return;
    }
    gl_FragColor = borderColor;
}
