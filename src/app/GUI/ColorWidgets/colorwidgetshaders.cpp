#include "colorwidgetshaders.h"
#include "glhelpers.h"
#include "exceptions.h"

ColorProgram HUE_PROGRAM;
ColorProgram HSV_SATURATION_PROGRAM;
ColorProgram VALUE_PROGRAM;

ColorProgram HSL_SATURATION_PROGRAM;
ColorProgram LIGHTNESS_PROGRAM;

ColorProgram RED_PROGRAM;
ColorProgram GREEN_PROGRAM;
ColorProgram BLUE_PROGRAM;

ColorProgram ALPHA_PROGRAM;

PlainColorProgram PLAIN_PROGRAM;
GradientProgram GRADIENT_PROGRAM;

BorderProgram BORDER_PROGRAM;
DoubleBorderProgram DOUBLE_BORDER_PROGRAM;

void iniColorProgram(QGL33 * const gl,
                     ColorProgram& program,
                     const QString& vShaderPath,
                     const QString& fShaderPath) {
    try {
        iniProgram(gl, program.fID, vShaderPath, fShaderPath);
        program.fHSVColorLoc = gl->glGetUniformLocation(
                    program.fID, "HSVColor"); // optional
        program.fRGBColorLoc = gl->glGetUniformLocation(
                    program.fID, "RGBColor"); // optional
        program.fHSLColorLoc = gl->glGetUniformLocation(
                    program.fID, "HSLColor"); // optional
        program.fCurrentValueLoc = gl->glGetUniformLocation(
                    program.fID, "currentValue");
        CheckInvalidLocation(program.fCurrentValueLoc, "currentValue");
        program.fHandleWidthLoc = gl->glGetUniformLocation(
                    program.fID, "handleWidth");
        CheckInvalidLocation(program.fHandleWidthLoc, "handleWidth");
        program.fLightHandleLoc = gl->glGetUniformLocation(
                    program.fID, "lightHandle");
        CheckInvalidLocation(program.fLightHandleLoc, "lightHandle");

        program.fMeshSizeLoc = gl->glGetUniformLocation(
                    program.fID, "meshSize"); // optional
    } catch(...) {
        RuntimeThrow("Error initializing color program.");
    }
}

void iniPlainColorProgram(QGL33 * const gl, const QString& colorShadersPath) {
    try {
        iniProgram(gl, PLAIN_PROGRAM.fID, GL_PLAIN_VERT,
                   colorShadersPath + "plain.frag");
        PLAIN_PROGRAM.fRGBAColorLoc = gl->glGetUniformLocation(
                    PLAIN_PROGRAM.fID, "RGBAColor");
        CheckInvalidLocation(PLAIN_PROGRAM.fRGBAColorLoc, "RGBAColor");

        PLAIN_PROGRAM.fMeshSizeLoc = gl->glGetUniformLocation(
                    PLAIN_PROGRAM.fID, "meshSize");
        CheckInvalidLocation(PLAIN_PROGRAM.fMeshSizeLoc, "meshSize");
    } catch(...) {
        RuntimeThrow("Error initializing plain color program.");
    }
}

void iniBorderProgram(QGL33 * const gl, const QString& colorShadersPath) {
    try {
        iniProgram(gl, BORDER_PROGRAM.fID, GL_PLAIN_VERT,
                   colorShadersPath + "border.frag");
        BORDER_PROGRAM.fBorderSizeLoc = gl->glGetUniformLocation(
                    BORDER_PROGRAM.fID, "borderSize");
        CheckInvalidLocation(BORDER_PROGRAM.fBorderSizeLoc, "borderSize");
        BORDER_PROGRAM.fBorderColorLoc = gl->glGetUniformLocation(
                    BORDER_PROGRAM.fID, "borderColor");
        CheckInvalidLocation(BORDER_PROGRAM.fBorderColorLoc, "borderColor");
    } catch(...) {
        RuntimeThrow("Error initializing border program.");
    }
}

void iniDoubleBorderProgram(QGL33 * const gl, const QString& colorShadersPath) {
    try {
        iniProgram(gl, DOUBLE_BORDER_PROGRAM.fID, GL_PLAIN_VERT,
                   colorShadersPath + "doubleborder.frag");
        DOUBLE_BORDER_PROGRAM.fInnerBorderSizeLoc = gl->glGetUniformLocation(
                    DOUBLE_BORDER_PROGRAM.fID, "innerBorderSize");
        CheckInvalidLocation(DOUBLE_BORDER_PROGRAM.fInnerBorderSizeLoc,
                             "innerBorderSize");
        DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc = gl->glGetUniformLocation(
                    DOUBLE_BORDER_PROGRAM.fID, "innerBorderColor");
        CheckInvalidLocation(DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc,
                             "innerBorderColor");
        DOUBLE_BORDER_PROGRAM.fOuterBorderSizeLoc = gl->glGetUniformLocation(
                    DOUBLE_BORDER_PROGRAM.fID, "outerBorderSize");
        CheckInvalidLocation(DOUBLE_BORDER_PROGRAM.fOuterBorderSizeLoc,
                             "outerBorderSize");
        DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc = gl->glGetUniformLocation(
                    DOUBLE_BORDER_PROGRAM.fID, "outerBorderColor");
        CheckInvalidLocation(DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc,
                             "outerBorderColor");
    } catch(...) {
        RuntimeThrow("Error initializing double border program.");
    }
}

void iniGradientProgram(QGL33 * const gl, const QString& colorShadersPath) {
    try {
        iniProgram(gl, GRADIENT_PROGRAM.fID, GL_PLAIN_VERT,
                   colorShadersPath + "gradient.frag");
        GRADIENT_PROGRAM.fRGBAColor1Loc = gl->glGetUniformLocation(
                    GRADIENT_PROGRAM.fID, "RGBAColor1");
        CheckInvalidLocation(GRADIENT_PROGRAM.fMeshSizeLoc, "RGBAColor1");
        GRADIENT_PROGRAM.fRGBAColor2Loc = gl->glGetUniformLocation(
                    GRADIENT_PROGRAM.fID, "RGBAColor2");
        CheckInvalidLocation(GRADIENT_PROGRAM.fMeshSizeLoc, "RGBAColor2");
        GRADIENT_PROGRAM.fMeshSizeLoc = gl->glGetUniformLocation(
                    GRADIENT_PROGRAM.fID, "meshSize");
        CheckInvalidLocation(GRADIENT_PROGRAM.fMeshSizeLoc, "meshSize");
    } catch(...) {
        RuntimeThrow("Error initializing gradient program.");
    }
}

void iniColorPrograms(QGL33 * const gl) {
    const QString colorShadersPath = ":/colorwidgetshaders/";

    try {
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
        iniPlainColorProgram(gl, colorShadersPath);
        iniGradientProgram(gl, colorShadersPath);
        iniBorderProgram(gl, colorShadersPath);
        iniDoubleBorderProgram(gl, colorShadersPath);
    } catch(...) {
        RuntimeThrow("Error initializing color programs.");
    }
}
