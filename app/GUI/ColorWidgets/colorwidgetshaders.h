#ifndef COLORWIDGETSHADERS_H
#define COLORWIDGETSHADERS_H
#include "glhelpers.h"

struct ColorProgram {
    GLuint fID;
    GLint fHSVColorLoc;
    GLint fRGBColorLoc;
    GLint fHSLColorLoc;

    GLint fCurrentValueLoc;
    GLint fHandleWidthLoc;
    GLint fLightHandleLoc;
    GLint fMeshSizeLoc;
};

struct PlainColorProgram {
    GLuint fID;
    GLint fRGBAColorLoc;
    GLint fMeshSizeLoc;
};

struct GradientProgram {
    GLuint fID;
    GLint fRGBAColor1Loc;
    GLint fRGBAColor2Loc;
    GLint fMeshSizeLoc;
};

struct BorderProgram {
    GLuint fID;
    GLint fBorderSizeLoc;
    GLint fBorderColorLoc;
};

struct DoubleBorderProgram {
    GLuint fID;
    GLint fInnerBorderSizeLoc;
    GLint fInnerBorderColorLoc;
    GLint fOuterBorderSizeLoc;
    GLint fOuterBorderColorLoc;
};

extern ColorProgram HUE_PROGRAM;
extern ColorProgram HSV_SATURATION_PROGRAM;
extern ColorProgram VALUE_PROGRAM;

extern ColorProgram HSL_SATURATION_PROGRAM;
extern ColorProgram LIGHTNESS_PROGRAM;

extern ColorProgram RED_PROGRAM;
extern ColorProgram GREEN_PROGRAM;
extern ColorProgram BLUE_PROGRAM;

extern ColorProgram ALPHA_PROGRAM;

extern PlainColorProgram PLAIN_PROGRAM;
extern GradientProgram GRADIENT_PROGRAM;

extern BorderProgram BORDER_PROGRAM;
extern DoubleBorderProgram DOUBLE_BORDER_PROGRAM;

extern void iniColorPrograms(QGL33c * const gl);

#endif // COLORWIDGETSHADERS_H
