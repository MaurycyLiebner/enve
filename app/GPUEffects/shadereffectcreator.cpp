#include "shadereffectcreator.h"
#include "exceptions.h"
#include "shadereffect.h"
#include <QDomDocument>

QList<stdsptr<ShaderEffectCreator>> ShaderEffectCreator::sEffectCreators;

qsptr<Property> ShaderEffectCreator::create() const {
    auto rasterEffect = SPtrCreate(ShaderEffect)(
                fName, this, &fProgram, fProperties);
    return std::move(rasterEffect);
}

qreal attrToDouble(const QDomElement &elem,
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

int attrToInt(const QDomElement &elem,
              const QString& elemName,
              const QString& attr) {
    try {
        const QString valS = elem.attribute(attr);
        const int val = stringToInt(valS);
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
            const qreal minVal = attrToDouble(elem, name, "min");
            const qreal maxVal = attrToDouble(elem, name, "max");
            const qreal iniVal = attrToDouble(elem, name, "ini");
            const qreal stepVal = attrToDouble(elem, name, "step");
            const QString script = elem.attribute("glValue");

            QrealAnimatorUniformSpecifierCreator::sTestScript(script, name);
            propC = SPtrCreate(QrealAnimatorCreator)(
                        iniVal, minVal, maxVal, stepVal, name);
            uniC = SPtrCreate(QrealAnimatorUniformSpecifierCreator)(script);
        } catch(...) {
            RuntimeThrow("Error while parsing Animator '" + name +
                         "' of type " + type + ".");
        }
    } else if(type == "int") {
        try {
            const int minVal = attrToInt(elem, name, "min");
            const int maxVal = attrToInt(elem, name, "max");
            const int iniVal = attrToInt(elem, name, "ini");
            const int stepVal = attrToInt(elem, name, "step");
            const QString script = elem.attribute("glValue");

            IntAnimatorUniformSpecifierCreator::sTestScript(script, name);
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

stdsptr<ShaderEffectCreator> ShaderEffectCreator::sLoadFromFile(
        QGL33c * const gl, const QString &grePath) {
    QFile greFile(grePath);
    if(!greFile.exists())
        RuntimeThrow("GPURasterEffect source file does not exist.");
    const QFileInfo info(greFile);
    const QString fragPath = info.path() + "/" + info.completeBaseName() + ".frag";
    const QFile fragFile(fragPath);
    if(!fragFile.exists())
        RuntimeThrow("GPURasterEffect shader file (" +
                     fragPath + ") does not exist.");
    if(!greFile.open(QIODevice::ReadOnly))
        RuntimeThrow("GPURasterEffect source file could not be opened.");
    QDomDocument document;
    QString errMsg;
    if(!document.setContent(&greFile, &errMsg))
        RuntimeThrow("Error while parsing GPURasterEffect source:\n" + errMsg);
    greFile.close();

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

    ShaderEffectProgram program;
    try {
        program = ShaderEffectProgram::sCreateProgram(
                    gl, fragPath, propCs, uniCs);
    } catch(...) {
        RuntimeThrow("Could not create a program for GPURasterEffect '" +
                     effectName + "'");
    }
    auto rasterEffectCreator =
            SPtrCreate(ShaderEffectCreator)(grePath, effectName, propCs, program);
    sEffectCreators << rasterEffectCreator;

    return rasterEffectCreator;
}

stdsptr<ShaderEffectCreator>
    ShaderEffectCreator::sWithGrePath(const QString &grePath) {
    for(const auto& effectC : sEffectCreators) {
        if(effectC->fGrePath == grePath) return effectC;
    }
    return nullptr;
}

stdsptr<ShaderEffectCreator>
    ShaderEffectCreator::sWithGrePathAndCompatible(
        const QString &grePath, const QList<PropertyType> &props) {
    const auto pathBased = sWithGrePath(grePath);
    if(!pathBased) return nullptr;
    if(pathBased->compatible(props))
        return pathBased;
    return nullptr;
}

QList<stdsptr<ShaderEffectCreator>>
    ShaderEffectCreator::sWithName(const QString &name) {
    QList<stdsptr<ShaderEffectCreator>> named;
    for(const auto& effectC : sEffectCreators) {
        if(effectC->fName == name) named << effectC;
    }
    return named;
}

QList<stdsptr<ShaderEffectCreator>>
    ShaderEffectCreator::sWithNameAndCompatible(
        const QString &name, const QList<PropertyType> &props) {
    QList<stdsptr<ShaderEffectCreator>> comp;
    for(const auto& effectC : sEffectCreators) {
        if(effectC->fName != name) continue;
        if(effectC->compatible(props)) comp << effectC;
    }
    return comp;
}

QList<stdsptr<ShaderEffectCreator>>
    ShaderEffectCreator::sWithCompatibleProps(
        const QList<PropertyType> &props) {
    QList<stdsptr<ShaderEffectCreator>> comp;
    for(const auto& effectC : sEffectCreators) {
        if(effectC->compatible(props)) comp << effectC;
    }
    return comp;
}

QList<stdsptr<ShaderEffectCreator>>
    ShaderEffectCreator::sGetBestCompatibleEffects(const Identifier &id) {
    const auto pathBased = sWithGrePathAndCompatible(id.fGrePath, id.fTypes);
    if(pathBased) return QList<stdsptr<ShaderEffectCreator>>() << pathBased;
    const auto nameBased = sWithNameAndCompatible(id.fName, id.fTypes);
    if(!nameBased.isEmpty()) return nameBased;
    return sWithCompatibleProps(id.fTypes);
}
