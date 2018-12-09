#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
typedef QOpenGLFunctions_3_3_Core QGL33c;

//! @brief Creates a program, compiles, and attaches associated shaders.
extern void iniProgram(QGL33c* gl,
                       GLuint& program,
                       const std::string& vShaderPath,
                       const std::string& fShaderPath);

extern void iniTexturedVShaderData(QGL33c* gl);
extern void iniPlainVShaderData(QGL33c* gl);

extern std::string GL_PLAIN_VERT;
extern GLuint GL_PLAIN_SQUARE_VAO;
extern GLuint GL_PLAIN_SQUARE_VBO;

extern std::string GL_TEXTURED_VERT;
extern GLuint GL_TEXTURED_SQUARE_VAO;
extern GLuint GL_TEXTURED_SQUARE_VBO;

extern GLuint GL_BLUR_PROGRAM;

#endif // GLHELPERS_H
