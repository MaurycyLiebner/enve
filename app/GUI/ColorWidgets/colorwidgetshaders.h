#ifndef COLORWIDGETSHADERS_H
#define COLORWIDGETSHADERS_H
#include "glhelpers.h"

extern GLuint HUE_PROGRAM;
extern GLuint HSV_SATURATION_PROGRAM;
extern GLuint VALUE_PROGRAM;

extern GLuint HSL_SATURATION_PROGRAM;
extern GLuint LIGHTNESS_PROGRAM;

extern GLuint RED_PROGRAM;
extern GLuint GREEN_PROGRAM;
extern GLuint BLUE_PROGRAM;

extern GLuint ALPHA_PROGRAM;

extern GLuint COLOR_WIDGET_VAO;
extern GLuint COLOR_WIDGET_VBO;

extern void iniColorPrograms(QGL33c* gl);

#endif // COLORWIDGETSHADERS_H
