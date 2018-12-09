#include "colorwidgetshaders.h"
#include "glhelpers.h"

void iniColorPrograms(QGL33c *gl) {
    std::string shadersPath =
            "/home/ailuropoda/Dev/AniVect/src/app/GUI/ColorWidgets/";
    iniProgram(gl, HUE_PROGRAM, GL_PLAIN_VERT
               shadersPath + "hue.frag");
    iniProgram(gl, HSV_SATURATION_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "hsv_saturation.frag");
    iniProgram(gl, VALUE_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "value.frag");
    iniProgram(gl, HSL_SATURATION_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "hsl_saturation.frag");
    iniProgram(gl, LIGHTNESS_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "lightness.frag");
    iniProgram(gl, RED_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "red.frag");
    iniProgram(gl, GREEN_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "green.frag");
    iniProgram(gl, BLUE_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "blue.frag");

    iniProgram(gl, BLUE_PROGRAM, GL_PLAIN_VERT,
               shadersPath + "alpha.frag");
}
