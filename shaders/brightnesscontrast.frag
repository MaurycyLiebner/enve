#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform float brightness;
uniform float contrast;

void main(void) {
    vec4 color = texture2D(texture, texCoord);
    gl_FragColor = vec4((color.xyz - 0.5) * contrast + 0.5 + brightness, color.w);
}
