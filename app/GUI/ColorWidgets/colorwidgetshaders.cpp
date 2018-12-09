#include "colorwidgetshaders.h"
#include "glhelpers.h"

void iniColorPrograms(QGL33c *gl) {
    std::string colorShadersPath =
            "/home/ailuropoda/Dev/AniVect/src/app/GUI/"
            "ColorWidgets/ColorWidgetShaders/";
    iniProgram(gl, HUE_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "hue.frag");
    iniProgram(gl, HSV_SATURATION_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "hsv_saturation.frag");
    iniProgram(gl, VALUE_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "value.frag");
    iniProgram(gl, HSL_SATURATION_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "hsl_saturation.frag");
    iniProgram(gl, LIGHTNESS_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "lightness.frag");
    iniProgram(gl, RED_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "red.frag");
    iniProgram(gl, GREEN_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "green.frag");
    iniProgram(gl, BLUE_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "blue.frag");

    iniProgram(gl, BLUE_PROGRAM, GL_PLAIN_VERT,
               colorShadersPath + "alpha.frag");
}
