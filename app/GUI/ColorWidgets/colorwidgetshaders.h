#ifndef COLORWIDGETSHADERS_H
#define COLORWIDGETSHADERS_H
#include "glhelpers.h"

struct ColorProgram {
    GLuint fID;
    GLint fCurrentHSVAColorLoc;
    GLint fCurrentValueLoc;
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

extern GLuint COLOR_WIDGET_VAO;
extern GLuint COLOR_WIDGET_VBO;

extern void iniColorPrograms(QGL33c* gl);

#endif // COLORWIDGETSHADERS_H
