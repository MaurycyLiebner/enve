#ifndef GPURASTEREFFECTCREATOR_H
#define GPURASTEREFFECTCREATOR_H
#include "gpurastereffectprogram.h"

enum PropertyType {
    PTYPE_FLOAT,
    PTYPE_INT
};

struct GPURasterEffectCreator : public PropertyCreator {
protected:
    GPURasterEffectCreator(const QString& grePath, const QString& name,
                           const QList<stdsptr<PropertyCreator>>& propCs,
                           const GPURasterEffectProgram& program) :
        PropertyCreator(name), fGrePath(grePath),
        fProperties(propCs), fProgram(program) {}
public:
    friend class StdSelfRef;

    const QString fGrePath;
    const QList<stdsptr<PropertyCreator>> fProperties;
    GPURasterEffectProgram fProgram;

    bool compatible(const QList<PropertyType>& props) const {
        if(props.count() != fProperties.count()) return false;
        for(int i = 0; i < props.count(); i++) {
            const auto& iType = props.at(i);
            const auto prop = fProperties.at(i).get();
            if(iType == PTYPE_FLOAT) {
                const bool iCompatible =
                        dynamic_cast<QrealAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else if(iType == PTYPE_INT) {
                const bool iCompatible =
                        dynamic_cast<IntAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else return false;
        }
        return true;
    }

    void reloadProgram(QGL33c * const gl, const QString& fragPath) {
        if(!QFile(fragPath).exists()) return;
        GPURasterEffectProgram program;
        try {
            program = GPURasterEffectProgram::sCreateProgram(
                        gl, fragPath, fProperties,
                        fProgram.fUniformCreators);
        } catch(...) {
            RuntimeThrow("Failed to load a new version of '" + fragPath + "'");
        }
        gl->glDeleteProgram(fProgram.fId);
        fProgram = program;
    }

    qsptr<Property> create() const;

    GLint getUniformLocation(QGL33c * const gl, const GLuint& program) const {
        Q_UNUSED(gl);
        Q_UNUSED(program);
        Q_ASSERT(false);
    }

    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl, const QString& grePath);

    static stdsptr<GPURasterEffectCreator> sWithGrePath(
            const QString& grePath);

    static stdsptr<GPURasterEffectCreator> sWithGrePathAndCompatible(
            const QString& grePath,
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sWithName(
            const QString &name);

    static QList<stdsptr<GPURasterEffectCreator>> sWithNameAndCompatible(
            const QString &name,
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sWithCompatibleProps(
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sGetBestCompatibleEffects(
            const QString& grePath, const QString &name,
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
};

#endif // GPURASTEREFFECTCREATOR_H
