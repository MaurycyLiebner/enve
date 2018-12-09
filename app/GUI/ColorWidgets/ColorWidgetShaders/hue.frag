#version 330 core
in vec3 pos;

uniform vec4 currentHSVAColor;
uniform float currentValue;
uniform float handleWidth; // fraction of width
uniform bool lightHandle;

vec4 rgba2hsva(vec4 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec4(abs(q.z + (q.w - q.y) / (6.0 * d + e)),
                d / (q.x + e), q.x, c.a);
}

vec4 hsva2rgba(vec4 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return vec4(c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y), c.a);
}

void main(void) {
    float fragHue = 0.5f*(1.f + pos.x);
    if(abs(currentValue - fragHue) < handleWidth) {
        if(lightHandle) {
            gl_FragColor = vec4(1.f, 1.f, 1.f, 1.f);
        } else {
            gl_FragColor = vec4(0.f, 0.f, 0.f, 1.f);
        }
        return;
    }
    vec4 hsvaColor = vec4(fragHue, currentHSVAColor.y,
                          currentHSVAColor.z, currentHSVAColor.w);
    gl_FragColor = hsva2rgba(hsvaColor);
}
