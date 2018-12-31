#include "gpurastereffectcreator.h"
#include "exceptions.h"
#include "gpurastereffect.h"
#include <QDomDocument>

QList<stdsptr<GPURasterEffectCreator>> GPURasterEffectCreator::sEffectCreators;

qsptr<Property> GPURasterEffectCreator::create() const {
    auto rasterEffect = SPtrCreate(GPURasterEffect)(fProgram, fName);
    Q_FOREACH(const auto& property, fProperties) {
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
        QString errMsg = "Can not convert '" + str + "' to double.";
        RuntimeThrow(errMsg.toStdString());
    }
    return val;
}

int stringToInt(const QString& str) {
    if(str.isEmpty()) {
        RuntimeThrow("Can not convert an empty string to int.");
    }
    bool ok;
    int val = str.toInt(&ok);
    if(!ok) {
        QString errMsg = "Can not convert '" + str + "' to int.";
        RuntimeThrow(errMsg.toStdString());
    }
    return val;
}

qreal tryConvertingAttrToDouble(const QDomElement &elem,
                                const QString& elemName,
                                const QString& attr) {
    try {
        QString valS = elem.attribute(attr);
        qreal val = stringToDouble(valS);
        return val;
    } catch(...) {
        QString errMsg = "Invalid " + attr + " value for " + elemName + ".";
        RuntimeThrow(errMsg.toStdString());
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
        QString errMsg = "Invalid " + attr + " value for " + elemName + ".";
        RuntimeThrow(errMsg.toStdString());
    }
}

void readAnimatorCreators(
        const QDomElement &elem,
        stdsptr<PropertyCreator>& propC,
        stdsptr<UniformSpecifierCreator>& uniC) {
    QString name = elem.attribute("name");
    if(name.isEmpty()) {
        RuntimeThrow("Animator name not defined.");
    }
    QRegExp rx("[A-Za-z0-9_]*");
    if(!rx.exactMatch(name)) {
        RuntimeThrow("Invalid Animator name.");
    }

    QString type = elem.attribute("type");
    if(type.isEmpty()) {
        QString errMsg = "Animator type not defined for " + name + ".";
        RuntimeThrow(errMsg.toStdString());
    }
    if(type == "float") {
        try {
            qreal minVal = tryConvertingAttrToDouble(elem, name, "min");
            qreal maxVal = tryConvertingAttrToDouble(elem, name, "max");
            qreal iniVal = tryConvertingAttrToDouble(elem, name, "ini");
            qreal stepVal = tryConvertingAttrToDouble(elem, name, "step");
            QString script = elem.attribute("glValue");

            propC = SPtrCreate(QrealAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            uniC = SPtrCreate(QrealAnimatorUniformSpecifierCreator)(script);
        } catch(...) {
            QString errMsg = "Error while parsing Animator '" + name + "' of type " + type + ".";
            RuntimeThrow(errMsg.toStdString());
        }
    } else if(type == "int") {
        try {
            int minVal = tryConvertingAttrToInt(elem, name, "min");
            int maxVal = tryConvertingAttrToInt(elem, name, "max");
            int iniVal = tryConvertingAttrToInt(elem, name, "ini");
            int stepVal = tryConvertingAttrToInt(elem, name, "step");
            QString script = elem.attribute("glValue");

            propC = SPtrCreate(IntAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            uniC = SPtrCreate(IntAnimatorUniformSpecifierCreator)(script);
        } catch(...) {
            QString errMsg = "Error while parsing Animator '" + name + "' of type " + type + ".";
            RuntimeThrow(errMsg.toStdString());
        }
    } else {
        QString errMsg = "Invalid Animator type '" + type + "' for " + name + ".";
        RuntimeThrow(errMsg.toStdString());
    }
}

stdsptr<GPURasterEffectCreator> GPURasterEffectCreator::sLoadFromFile(
        QGL33c * const gl, const QString &filePath) {
    QFile file(filePath);
    if(!file.exists()) {
        RuntimeThrow("GPURasterEffect source file does not exist.");
    }
    QFileInfo info(file);
    QString fragPath = info.path() + "/" + info.completeBaseName() + ".frag";
    QFile fragFile(fragPath);
    if(!fragFile.exists()) {
        QString errMsg = "GPURasterEffect shader file (" + fragPath + ") does not exist.";
        RuntimeThrow(errMsg.toStdString());
    }
    if(!file.open(QIODevice::ReadOnly)) {
        RuntimeThrow("GPURasterEffect source file could not be opened.");
    }
    QDomDocument document;
    QString errMsg;
    if(!document.setContent(&file, &errMsg)) {
        errMsg = "Error while parsing GPURasterEffect source:\n" + errMsg;
        RuntimeThrow(errMsg.toStdString());
    }
    file.close();

    QDomElement root = document.firstChildElement();
    if(root.tagName() != "GPURasterEffect") {
        errMsg = "Unrecogized root " + root.tagName() +
                "in GPURasterEffect source.";
        RuntimeThrow(errMsg.toStdString());
    }
    QString effectName = root.attribute("name");

    QList<stdsptr<PropertyCreator>> propCs;
    UniformSpecifierCreators uniCs;
    QDomNodeList animatorNodes = root.elementsByTagName("Animator");
    for(int i = 0; i < animatorNodes.count(); i++) {
        QDomNode animNode = animatorNodes.at(i);
        if(!animNode.isElement()) {
            errMsg = "Animator node " + QString::number(i) + " is not an Element.";
            RuntimeThrow(errMsg.toStdString());
        }
        QDomElement animEle = animNode.toElement();
        try {
            stdsptr<PropertyCreator> propC;
            stdsptr<UniformSpecifierCreator> uniC;
            readAnimatorCreators(animEle, propC, uniC);
            propCs << propC;
            uniCs << uniC;
        } catch(...) {
            errMsg = "Animator " + QString::number(i) + " is invalid.";
            RuntimeThrow(errMsg.toStdString());
        }
    }

    GPURasterEffectProgram program;
    try {
        iniProgram(gl, program.fID, GL_TEXTURED_VERT, fragPath.toStdString());
    } catch(...) {
        errMsg = "Could not create program for GPURasterEffect '" + effectName + "'.";
        RuntimeThrow(errMsg.toStdString());
    }

    foreach(const auto& propC, propCs) {
        GLint loc = propC->getUniformLocation(gl, program.fID);
        if(loc < 0) {
            gl->glDeleteProgram(program.fID);
            errMsg = "'" + propC->fName + "' does not correspond to an active uniform variable.";
            RuntimeThrow(errMsg.toStdString());
        }
        program.fArgumentLocs.append(loc);
    }
    program.fGPosLoc = gl->glGetUniformLocation(program.fID, "_gPos");
    program.fUniformCreators = uniCs;
    program.fTexLocation = gl->glGetUniformLocation(program.fID, "texture");
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
    auto qa = GetAsPtr(property, QrealAnimator);
    QString propName = property->prp_getName();
    qreal val = qa->getCurrentEffectiveValueAtRelFrameF(relFrame);

    if(mScript.isEmpty()) {
        return [&loc, &val](QGL33c * const gl, QJSEngine&) {
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
                    QString errMsg = "Invalid glValue script for '" + propName + "'.";
                    RuntimeThrow(errMsg.toStdString());
                }
            } else {
                QString errMsg = "Invalid glValue script for '" + propName + "'.";
                RuntimeThrow(errMsg.toStdString());
            }
        };
    }
}
