#version 330 core
layout (location = 0) in vec3 vPos;

out vec3 pos;

void main(void) {
    gl_Position = vec4(vPos, 1.f);
    pos = vPos;
}
