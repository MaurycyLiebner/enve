#version 330 core
in vec2 texCoord;
layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform sampler2D texture;
uniform vec2 translate;
uniform float dotDistance;
uniform float dotRadius;

void main(void) {
    bool inDot;
    float mixAlpha;

    vec2 transformedCoord = translate + gl_FragCoord.xy;

    vec2 dotID = floor(transformedCoord/(2.f*(dotRadius + dotDistance)));
    vec2 posInDot = transformedCoord - dotID*2.f*(dotDistance + dotRadius);
    vec2 posRelDotCenter = posInDot - dotRadius;
    float distToCenter = sqrt(posRelDotCenter.x*posRelDotCenter.x +
                              posRelDotCenter.y*posRelDotCenter.y);
    float distToEdge = distToCenter - sqrt(dotRadius*dotRadius);
    if(distToEdge > 0.f) {
        inDot = false;
    } else {
        distToEdge = abs(distToEdge);
        inDot = true;
        if(distToEdge < 1.f) {
            mixAlpha = distToEdge;
        } else {
            mixAlpha = 1.f;
        }
    }
    if(inDot) {
        vec4 texCol = texture2D(texture, texCoord);
        gl_FragColor =  vec4(texCol.xyz, texCol.w*mixAlpha);
    } else {
        gl_FragColor =  vec4(0.f, 0.f, 0.f, 0.f);
    }
}
