#include "gpurastereffectcreator.h"
#include "exceptions.h"
#include "gpurastereffect.h"
#include <QDomDocument>

QList<stdsptr<GPURasterEffectCreator>> GPURasterEffectCreator::sEffectCreators;

qsptr<Property> GPURasterEffectCreator::create() const {
    auto rasterEffect = SPtrCreate(GPURasterEffect)(fProgram, fName);
    for(const auto& property : fProperties) {
        rasterEffect->ca_addChildAnimator(property->create());
    }
    return std::move(rasterEffect);
}

qreal stringToDouble(const QString& str) {
    if(str.isEmpty()) {
        RuntimeThrow("Can not convert an empty string to double.");
    }
    bool ok;
    qreal val = str.toDouble(&ok);
    if(!ok) {
        RuntimeThrow("Can not convert '" + str + "' to double.");
    }
    return val;
}

int stringToInt(const QString& str) {
    if(str.isEmpty()) RuntimeThrow("Can not convert an empty string to int.");
    bool ok;
    int val = str.toInt(&ok);
    if(!ok) RuntimeThrow("Can not convert '" + str + "' to int.");
    return val;
}

qreal tryConvertingAttrToDouble(const QDomElement &elem,
                                const QString& elemName,
                                const QString& attr) {
    try {
        const QString valS = elem.attribute(attr);
        const qreal val = stringToDouble(valS);
        return val;
    } catch(...) {
        RuntimeThrow("Invalid " + attr + " value for " + elemName + ".");
    }
}

int tryConvertingAttrToInt(const QDomElement &elem,
                           const QString& elemName,
                           const QString& attr) {
    try {
        QString valS = elem.attribute(attr);
        int val = stringToInt(valS);
        return val;
    } catch(...) {
        RuntimeThrow("Invalid " + attr + " value for " + elemName + ".");
    }
}

void readAnimatorCreators(
        const QDomElement &elem,
        stdsptr<PropertyCreator>& propC,
        stdsptr<UniformSpecifierCreator>& uniC) {
    const QString name = elem.attribute("name");
    if(name.isEmpty()) RuntimeThrow("Animator name not defined.");
    const QRegExp rx("[A-Za-z0-9_]*");
    if(!rx.exactMatch(name)) RuntimeThrow("Invalid Animator name.");

    const QString type = elem.attribute("type");
    if(type.isEmpty())
        RuntimeThrow("Animator type not defined for " + name + ".");
    if(type == "float") {
        try {
            const qreal minVal = tryConvertingAttrToDouble(elem, name, "min");
            const qreal maxVal = tryConvertingAttrToDouble(elem, name, "max");
            const qreal iniVal = tryConvertingAttrToDouble(elem, name, "ini");
            const qreal stepVal = tryConvertingAttrToDouble(elem, name, "step");
            const QString script = elem.attribute("glValue");

            propC = SPtrCreate(QrealAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            uniC = SPtrCreate(QrealAnimatorUniformSpecifierCreator)(script);
        } catch(...) {
            RuntimeThrow("Error while parsing Animator '" + name +
                         "' of type " + type + ".");
        }
    } else if(type == "int") {
        try {
            const int minVal = tryConvertingAttrToInt(elem, name, "min");
            const int maxVal = tryConvertingAttrToInt(elem, name, "max");
            const int iniVal = tryConvertingAttrToInt(elem, name, "ini");
            const int stepVal = tryConvertingAttrToInt(elem, name, "step");
            const QString script = elem.attribute("glValue");

            propC = SPtrCreate(IntAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            uniC = SPtrCreate(IntAnimatorUniformSpecifierCreator)(script);
        } catch(...) {
            RuntimeThrow("Error while parsing Animator '" + name +
                         "' of type " + type + ".");
        }
    } else {
        RuntimeThrow("Invalid Animator type '" + type + "' for " + name + ".");
    }
}

stdsptr<GPURasterEffectCreator> GPURasterEffectCreator::sLoadFromFile(
        QGL33c * const gl, const QString &filePath) {
    QFile file(filePath);
    if(!file.exists())
        RuntimeThrow("GPURasterEffect source file does not exist.");
    const QFileInfo info(file);
    const QString fragPath = info.path() + "/" + info.completeBaseName() + ".frag";
    const QFile fragFile(fragPath);
    if(!fragFile.exists())
        RuntimeThrow("GPURasterEffect shader file (" +
                     fragPath + ") does not exist.");
    if(!file.open(QIODevice::ReadOnly))
        RuntimeThrow("GPURasterEffect source file could not be opened.");
    QDomDocument document;
    QString errMsg;
    if(!document.setContent(&file, &errMsg))
        RuntimeThrow("Error while parsing GPURasterEffect source:\n" + errMsg);
    file.close();

    QDomElement root = document.firstChildElement();
    if(root.tagName() != "GPURasterEffect")
        RuntimeThrow("Unrecogized root " + root.tagName() +
                     "in GPURasterEffect source.");
    const QString effectName = root.attribute("name");

    QList<stdsptr<PropertyCreator>> propCs;
    UniformSpecifierCreators uniCs;
    QDomNodeList animatorNodes = root.elementsByTagName("Animator");
    for(int i = 0; i < animatorNodes.count(); i++) {
        QDomNode animNode = animatorNodes.at(i);
        if(!animNode.isElement()) {
            RuntimeThrow("Animator node " + QString::number(i) +
                         " is not an Element.");
        }
        QDomElement animEle = animNode.toElement();
        try {
            stdsptr<PropertyCreator> propC;
            stdsptr<UniformSpecifierCreator> uniC;
            readAnimatorCreators(animEle, propC, uniC);
            propCs << propC;
            uniCs << uniC;
        } catch(...) {
            RuntimeThrow("Animator " + QString::number(i) + " is invalid.");
        }
    }

    GPURasterEffectProgram program;
    try {
        iniProgram(gl, program.fId, GL_TEXTURED_VERT, fragPath);
    } catch(...) {
        RuntimeThrow("Could not create program for GPURasterEffect '" +
                     effectName + "'.");
    }

    for(const auto& propC : propCs) {
        GLint loc = propC->getUniformLocation(gl, program.fId);
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

    auto rasterEffectCreator = SPtrCreate(GPURasterEffectCreator)(effectName);
    rasterEffectCreator->fProgram = program;
    rasterEffectCreator->fProperties = propCs;
    sEffectCreators << rasterEffectCreator;

    return rasterEffectCreator;
}

UniformSpecifier QrealAnimatorUniformSpecifierCreator::create(
        const GLint &loc,
        Property * const property,
        const qreal &relFrame) const {
    const auto qa = GetAsPtr(property, QrealAnimator);
    const QString propName = property->prp_getName();
    const qreal val = qa->getEffectiveValue(relFrame);

    if(mScript.isEmpty()) {
        return [loc, val](QGL33c * const gl, QJSEngine&) {
            gl->glUniform1f(loc, static_cast<GLfloat>(val));
        };
    } else {
        QString script = mScript;
        return [loc, val, script, propName](
                QGL33c * const gl, QJSEngine& engine) {
            QString valScript = propName + " = " + QString::number(val);
            engine.evaluate(valScript);
            QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isNumber()) {
                gl->glUniform1f(loc, static_cast<GLfloat>(jsVal.toNumber()));
            } else if(jsVal.isArray()) {
                QStringList vals = jsVal.toString().split(',');
                if(vals.count() == 1) {
                    double val1 = stringToDouble(vals.first());
                    gl->glUniform1f(loc, static_cast<GLfloat>(val1));
                } else if(vals.count() == 2) {
                    double val1 = stringToDouble(vals.first());
                    double val2 = stringToDouble(vals.last());

                    gl->glUniform2f(loc, static_cast<GLfloat>(val1),
                                    static_cast<GLfloat>(val2));
                } else {
                    RuntimeThrow("Invalid glValue script for '" + propName + "'.");
                }
            } else {
                RuntimeThrow("Invalid glValue script for '" + propName + "'.");
            }
        };
    }
}
