#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform float brightness;
uniform float contrast;

vec4 colorPaletteAlt(vec4 inColor, sampler2D texPalette, sampler2D texPaletteAlt) {
    vec4 outColor = vec4(0.0);
    float minDis = 1000.0;
    float dis = 100.0;
    if(inColor.a > 0.001) {
        for(int i = 0; i < SAMPLES; i++) {
            vec4 palleteColor = texture2D(texPalette, vec2((float(i) + 0.5) / float(SAMPLES), 0.5)); //Map to HSL Space
            dis = sqrDistance(rgbToHsl(inColor).rgb,rgbToHsl(palleteColor).rgb);
            if(palleteColor.a > 0.001) {
                if(dis < minDis) {
                    minDis = dis;
                    outColor = texture2D(texPaletteAlt, vec2((float(i) + 0.5) / float(SAMPLES), 0.5));
                }
            }
        }
    }
    return outColor;
}

void main(void) {
    vec4 color = texture2D(texture, texCoord);
    gl_FragColor = vec4((color.xyz - 0.5) * contrast + 0.5 + brightness, color.w);
}
