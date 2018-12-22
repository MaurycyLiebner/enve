#include "gpurastereffect.h"
#include "exceptions.h"
#include "Animators/qrealanimator.h"
#include <QDomDocument>
QList<stdsptr<GPURasterEffectCreator>> GPURasterEffectCreator::sEffectCreators;

qsptr<Property> GPURasterEffectCreator::create() const {
    auto rasterEffect = SPtrCreate(GPURasterEffect)(fName);
    Q_FOREACH(const auto& property, fProperties) {
        rasterEffect->ca_addChildAnimator(property->create());
    }
    return std::move(rasterEffect);
}

GPURasterEffect::GPURasterEffect(const QString& name) :
    ComplexAnimator(name) {

}

qreal stringToDouble(const QString& str) {
    if(str.isEmpty()) {
        RuntimeThrow("Can not convert an empty string to float.");
    }
    bool ok;
    qreal val = str.toDouble(&ok);
    if(!ok) {
        QString errMsg = "Can not convert '" + str + "' to float.";
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

stdsptr<QrealAnimatorCreator> readAnimatorCreator(const QDomElement &elem) {
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

            auto creator = SPtrCreate(QrealAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            return creator;
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

    QList<stdsptr<PropertyCreator>> properties;
    QDomNodeList animatorNodes = root.elementsByTagName("Animator");
    for(int i = 0; i < animatorNodes.count(); i++) {
        QDomNode animNode = animatorNodes.at(i);
        if(!animNode.isElement()) {
            errMsg = "Animator node " + QString::number(i) + " is not an Element.";
            RuntimeThrow(errMsg.toStdString());
        }
        QDomElement animEle = animNode.toElement();
        try {
            auto anim = readAnimatorCreator(animEle);
            properties << anim;
        } catch(...) {
            errMsg = "Animator " + QString::number(i) + " is invalid.";
            RuntimeThrow(errMsg.toStdString());
        }
    }

    GLuint program;
    try {
        iniProgram(gl, program, GL_TEXTURED_VERT, fragPath.toStdString());
    } catch(...) {
        errMsg = "Could not create program for GPURasterEffect '" + effectName + "'.";
        RuntimeThrow(errMsg.toStdString());
    }

    auto rasterEffectCreator = SPtrCreate(GPURasterEffectCreator)(effectName);
    rasterEffectCreator->fProperties = properties;
    sEffectCreators << rasterEffectCreator;

    return rasterEffectCreator;
}
