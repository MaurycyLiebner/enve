#ifndef SHADEREFFECTPROGRAM_H
#define SHADEREFFECTPROGRAM_H
#include "uniformspecifiercreator.h"
#include "shadervaluehandler.h"

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct ShaderEffectProgram {
    GLuint fId = 0;
    GLuint fFragShader;
    GLint fGPosLoc;
    GLint fTexLocation;
    QList<GLint> fPropUniLocs;
    UniformSpecifierCreators fPropUniCreators;
    QList<stdsptr<ShaderValueHandler>> fValueHandlers;
    QList<GLint> fValueLocs;
    QString fMarginScript;

    static ShaderEffectProgram sCreateProgram(
            QGL33 * const gl, const QString &fragPath,
            const QString& marginScript,
            const QList<stdsptr<ShaderPropertyCreator>>& propCs,
            const UniformSpecifierCreators& uniCs,
            const QList<stdsptr<ShaderValueHandler>>& values);
};

#endif // SHADEREFFECTPROGRAM_H
