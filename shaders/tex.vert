#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexCoord;

out vec2 texCoord;

void main(void) {
    gl_Position = vec4(vPos, 1.f);
    texCoord = vTexCoord;
}
