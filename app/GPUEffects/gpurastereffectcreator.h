#ifndef GPURASTEREFFECTCREATOR_H
#define GPURASTEREFFECTCREATOR_H
#include "Animators/qrealanimatorcreator.h"
#include "Animators/intanimatorcreator.h"
#include <QJSEngine>

typedef std::function<void(QGL33c * const, QJSEngine&)> UniformSpecifier;
typedef QList<UniformSpecifier> UniformSpecifiers;
struct UniformSpecifierCreator : public StdSelfRef {
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const = 0;
};

struct QrealAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    QrealAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const;

    static void sTestScript(const QString& script, const QString& propName);
private:
    QString mScript;
};

struct IntAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    IntAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const;

    static void sTestScript(const QString& script, const QString& propName);
private:
    QString mScript;
};

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct GPURasterEffectProgram {
    GLuint fId;
    GLint fGPosLoc;
    GLint fTexLocation;
    QList<GLint> fArgumentLocs;
    UniformSpecifierCreators fUniformCreators;

    void cleanUp(QGL33c * const gl) {
        gl->glDeleteProgram(fId);
    }

    static GPURasterEffectProgram sCreateProgram(
            QGL33c * const gl, const QString &fragPath,
            const QList<stdsptr<PropertyCreator>>& propCs,
            const UniformSpecifierCreators& uniCs) {
        GPURasterEffectProgram program;
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

struct GPURasterEffectCreator : public PropertyCreator {
    friend class StdSelfRef;

    QList<stdsptr<PropertyCreator>> fProperties;
    GPURasterEffectProgram fProgram;

    void reloadProgram(QGL33c * const gl, const QString& fragPath) {
        if(!QFile(fragPath).exists()) return;
        GPURasterEffectProgram program;
        try {
            program = GPURasterEffectProgram::sCreateProgram(
                        gl, fragPath, fProperties, fProgram.fUniformCreators);
        } catch(...) {
            RuntimeThrow("Failed to load a new version of '" + fragPath + "'");
        }
        fProgram.cleanUp(gl);
        fProgram = program;
    }

    qsptr<Property> create() const;

    GLint getUniformLocation(QGL33c * const gl, const GLuint& program) const {
        Q_UNUSED(gl);
        Q_UNUSED(program);
        Q_ASSERT(false);
    }

    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl,
            const QString& filePath);

    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
protected:
    GPURasterEffectCreator(const QString& name) : PropertyCreator(name) {}
};

#endif // GPURASTEREFFECTCREATOR_H
