#include "colorwidgetshaders.h"
#include "glhelpers.h"

ColorProgram HUE_PROGRAM;
ColorProgram HSV_SATURATION_PROGRAM;
ColorProgram VALUE_PROGRAM;

ColorProgram HSL_SATURATION_PROGRAM;
ColorProgram LIGHTNESS_PROGRAM;

ColorProgram RED_PROGRAM;
ColorProgram GREEN_PROGRAM;
ColorProgram BLUE_PROGRAM;

ColorProgram ALPHA_PROGRAM;

GLuint COLOR_WIDGET_VAO;
GLuint COLOR_WIDGET_VBO;

void iniColorProgram(QGL33c* gl,
                     ColorProgram& program,
                     const std::string& vShaderPath,
                     const std::string& fShaderPath) {
    iniProgram(gl, program.fID, vShaderPath, fShaderPath);
    gl->glUseProgram(program.fID);
    assertNoGlErrors();
    program.fCurrentHSVAColorLoc = gl->glGetUniformLocation(
                program.fID, "currentHSVAColor");
    assertNoGlErrors();
    assert(program.fCurrentHSVAColorLoc >= 0);
    program.fCurrentValueLoc = gl->glGetUniformLocation(
                program.fID, "currentValue");
    assertNoGlErrors();
    assert(program.fCurrentValueLoc >= 0);
    program.fHandleWidthLoc = gl->glGetUniformLocation(
                program.fID, "handleWidth");
    assertNoGlErrors();
    assert(program.fHandleWidthLoc >= 0);
    program.fLightHandle = gl->glGetUniformLocation(
                program.fID, "lightHandle");
    assertNoGlErrors();
    assert(program.fLightHandle >= 0);
}

void iniColorPrograms(QGL33c *gl) {
    std::string colorShadersPath =
            "/home/ailuropoda/Dev/AniVect/src/app/GUI/"
            "ColorWidgets/ColorWidgetShaders/";
    iniColorProgram(gl, HUE_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "hue.frag");
    iniColorProgram(gl, HSV_SATURATION_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "hsv_saturation.frag");
    iniColorProgram(gl, VALUE_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "value.frag");
    iniColorProgram(gl, HSL_SATURATION_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "hsl_saturation.frag");
    iniColorProgram(gl, LIGHTNESS_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "lightness.frag");
    iniColorProgram(gl, RED_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "red.frag");
    iniColorProgram(gl, GREEN_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "green.frag");
    iniColorProgram(gl, BLUE_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "blue.frag");

    iniColorProgram(gl, ALPHA_PROGRAM, GL_PLAIN_VERT,
                    colorShadersPath + "alpha.frag");
}
