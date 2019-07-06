#ifndef GPURASTEREFFECTPROGRAM_H
#define GPURASTEREFFECTPROGRAM_H
#include "uniformspecifiercreator.h"

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct ShaderEffectProgram {
    GLuint fId = 0;
    GLuint fFragShader;
    GLint fGPosLoc;
    GLint fTexLocation;
    QList<GLint> fArgumentLocs;
    UniformSpecifierCreators fUniformCreators;

    static ShaderEffectProgram sCreateProgram(
            QGL33c * const gl, const QString &fragPath,
            const QList<stdsptr<PropertyCreator>>& propCs,
            const UniformSpecifierCreators& uniCs) {
        ShaderEffectProgram program;
        try {
            iniProgram(gl, program.fId, GL_TEXTURED_VERT, fragPath);
        } catch(...) {
            RuntimeThrow("Could not initialize a program for GPURasterEffect");
        }

        for(const auto& propC : propCs) {
            const GLint loc = propC->getUniformLocation(gl, program.fId);
            if(loc < 0) {
                gl->glDeleteProgram(program.fId);
                RuntimeThrow("'" + propC->fName +
                             "' does not correspond to an active uniform variable.");
            }
            program.fArgumentLocs.append(loc);
        }
        program.fGPosLoc = gl->glGetUniformLocation(program.fId, "_gPos");
        program.fUniformCreators = uniCs;
        program.fTexLocation = gl->glGetUniformLocation(program.fId, "texture");
        CheckInvalidLocation(program.fTexLocation, "texture");
        return program;
    }
};

#endif // GPURASTEREFFECTPROGRAM_H
