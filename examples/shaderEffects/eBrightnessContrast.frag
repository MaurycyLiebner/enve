#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform float brightness;
uniform float contrast;

void main(void) {
    vec4 color = texture2D(texture, texCoord);
    gl_FragColor = vec4((color.rgb - 0.5*color.a) * (contrast + 1) + 0.5*color.a + brightness*color.a, color.a);
}
