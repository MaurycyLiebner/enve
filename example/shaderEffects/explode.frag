#version 330 core
in vec2 texCoord;

uniform sampler2D texture;
uniform vec2 displacementXY;
uniform int steps;
uniform int directions;

const float PIx2 = 6.28318530718;

void main(void) {
    vec4 Color = texture2D(texture, texCoord);
    for(float d = 0.0; d < PIx2; d += PIx2/float(directions)) {
        for(float i= 1.0/float(steps); i <= 1.0; i += 1.0/float(steps)) {
            Color += texture2D(texture, texCoord + vec2(cos(d), sin(d))*displacementXY*i);
        }
    }
    Color /= float(steps)*float(directions) + 1.0;
    gl_FragColor =  Color;
}
