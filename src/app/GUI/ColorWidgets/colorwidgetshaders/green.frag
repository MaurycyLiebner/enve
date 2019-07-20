#version 330 core
in vec3 pos;

uniform vec3 RGBColor;

uniform float currentValue;
uniform float handleWidth; // fraction of width
uniform bool lightHandle;

void main(void) {
    float fragGreen = 0.5f*(1.f + pos.x);
    if(abs(currentValue - fragGreen) < handleWidth) {
        if(lightHandle) {
            gl_FragColor = vec4(1.f, 1.f, 1.f, 1.f);
        } else {
            gl_FragColor = vec4(0.f, 0.f, 0.f, 1.f);
        }
        return;
    }

    gl_FragColor = vec4(RGBColor.x, fragGreen, RGBColor.z, 1.f);
}
