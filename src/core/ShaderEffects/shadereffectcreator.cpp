// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "shadereffectcreator.h"
#include "exceptions.h"
#include "shadereffect.h"
#include "shadervaluehandler.h"
#include <QDomDocument>

QList<stdsptr<ShaderEffectCreator>> ShaderEffectCreator::sEffectCreators;

qsptr<ShaderEffect> ShaderEffectCreator::create() const {
    auto shaderEffect = enve::make_shared<ShaderEffect>(
                fName, this, &fProgram, fProperties);
    return shaderEffect;
}

qreal stringToDouble(const QString &str) {
    if(str.isEmpty()) RuntimeThrow("Can not convert an empty string to double");
    bool ok;
    qreal val = str.toDouble(&ok);
    if(!ok) RuntimeThrow("Can not convert '" + str + "' to double");
    return val;
}

int stringToInt(const QString &str) {
    if(str.isEmpty()) RuntimeThrow("Can not convert an empty string to int");
    bool ok;
    int val = str.toInt(&ok);
    if(!ok) RuntimeThrow("Can not convert '" + str + "' to int");
    return val;
}

int stringToBool(const QString &str) {
    if(str.isEmpty()) RuntimeThrow("Can not convert an empty string to bool");
    const auto simpl = str.simplified();
    bool result;
    if(simpl == "true") result = true;
    else if(simpl == "false") result = false;
    else RuntimeThrow("Can not convert '" + str + "' to bool");
    return result;
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

bool attrToBool(const QDomElement &elem,
                const QString& elemName,
                const QString& attr,
                const QString& def) {
    try {
        const QString valS = elem.attribute(attr, def);
        const bool val = stringToBool(valS);
        return val;
    } catch(...) {
        RuntimeThrow("Invalid " + attr + " value for " + elemName + ".");
    }
}
void readAnimatorCreators(const QDomElement &elem,
                         stdsptr<ShaderPropertyCreator>& propC,
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
            const bool glValue = attrToBool(elem, name, "glValue", "false");
            const bool resolutionScaled = attrToBool(elem, name, "resolutionScaled", "false");

            propC = enve::make_shared<QrealAnimatorCreator>(
                        iniVal, minVal, maxVal, stepVal, glValue, name);
            uniC = enve::make_shared<UniformSpecifierCreator>(
                        ShaderPropertyType::qrealAnimator, glValue, resolutionScaled);
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
            const bool glValue = attrToBool(elem, name, "glValue", "false");

            propC = enve::make_shared<IntAnimatorCreator>(
                        iniVal, minVal, maxVal, stepVal, glValue, name);
            uniC = enve::make_shared<UniformSpecifierCreator>(
                        ShaderPropertyType::intAnimator, glValue, false);
        } catch(...) {
            RuntimeThrow("Error while parsing Animator '" + name +
                         "' of type " + type + ".");
        }
    } else {
        RuntimeThrow("Invalid Animator type '" + type + "' for " + name + ".");
    }
}

stdsptr<ShaderEffectCreator> ShaderEffectCreator::sLoadFromFile(
        QGL33 * const gl, const QString &grePath) {
    QFile greFile(grePath);
    if(!greFile.exists())
        RuntimeThrow("ShaderEffect source file does not exist.");
    const QFileInfo info(greFile);
    const QString fragPath = info.path() + "/" + info.completeBaseName() + ".frag";
    const QFile fragFile(fragPath);
    if(!fragFile.exists())
        RuntimeThrow("ShaderEffect shader file (" +
                     fragPath + ") does not exist.");
    if(!greFile.open(QIODevice::ReadOnly))
        RuntimeThrow("ShaderEffect source file could not be opened.");
    QDomDocument document;
    QString errMsg;
    if(!document.setContent(&greFile, &errMsg))
        RuntimeThrow("Error while parsing ShaderEffect source:\n" + errMsg);
    greFile.close();

    QDomElement root = document.firstChildElement();
    if(root.tagName() != "ShaderEffect")
        RuntimeThrow("Unrecogized root " + root.tagName() +
                     " in ShaderEffect source.");
    const QString effectName = root.attribute("name");

    QList<stdsptr<ShaderPropertyCreator>> propCs;
    UniformSpecifierCreators uniCs;
    const QDomNodeList animatorNodes = root.elementsByTagName("Animator");
    for(int i = 0; i < animatorNodes.count(); i++) {
        const QDomNode& animNode = animatorNodes.at(i);
        if(!animNode.isElement()) {
            RuntimeThrow("Animator node " + QString::number(i) +
                         " is not an Element.");
        }
        QDomElement animEle = animNode.toElement();
        try {
            stdsptr<ShaderPropertyCreator> propC;
            stdsptr<UniformSpecifierCreator> uniC;
            readAnimatorCreators(animEle, propC, uniC);
            propCs << propC;
            uniCs << uniC;
        } catch(...) {
            RuntimeThrow("Animator " + QString::number(i) + " is invalid.");
        }
    }

    QList<stdsptr<ShaderValueHandler>> values;
    const QDomNodeList valueNodes = root.elementsByTagName("Value");
    for(int i = 0; i < valueNodes.count(); i++) {
        const QDomNode& valNode = valueNodes.at(i);
        if(!valNode.isElement()) {
            RuntimeThrow("Value node " + QString::number(i) +
                         " is not an Element.");
        }
        const QDomElement valEle = valNode.toElement();
        try {
            const QString name = valEle.attribute("name");
            if(name.isEmpty()) RuntimeThrow("Value name not defined.");
            const auto typeStr = valEle.attribute("type");
            if(typeStr.isEmpty()) RuntimeThrow("Value '" + name + "' type not defined.");
            GLValueType type{GLValueType::Float};
            if(typeStr == "float") type = GLValueType::Float;
            else if(typeStr == "vec2") type = GLValueType::Vec2;
            else if(typeStr == "vec3") type = GLValueType::Vec3;
            else if(typeStr == "vec4") type = GLValueType::Vec4;
            else if(typeStr == "int") type = GLValueType::Int;
            else if(typeStr == "ivec2") type = GLValueType::iVec2;
            else if(typeStr == "ivec3") type = GLValueType::iVec3;
            else if(typeStr == "ivec4") type = GLValueType::iVec4;

            const QString script = valEle.attribute("value");
            if(typeStr.isEmpty()) RuntimeThrow("Value '" + name + "' value not defined.");

            const bool glValue = attrToBool(valEle, name, "glValue", "false");
            values << enve::make_shared<ShaderValueHandler>(name, glValue, type, script);
        } catch(...) {
            RuntimeThrow("Value " + QString::number(i) + " is invalid.");
        }
    }
    const QDomNodeList marginNode = root.elementsByTagName("Margin");
    QString marginScript;
    if(marginNode.count() == 1) {
        const QDomNode& marNode = marginNode.at(0);
        if(!marNode.isElement()) RuntimeThrow("Margin node is not an Element.");
        const QDomElement marEle = marNode.toElement();
        marginScript = marEle.attribute("value");
    } else if(marginNode.count() > 1) RuntimeThrow("Multiple 'Margin' definitions");


    ShaderEffectProgram program;
    try {
        program = ShaderEffectProgram::sCreateProgram(
                    gl, fragPath, marginScript, propCs, uniCs, values);
    } catch(...) {
        RuntimeThrow("Could not create a program for ShaderEffect '" +
                     effectName + "'");
    }
    const auto shaderEffectCreator = enve::make_shared<ShaderEffectCreator>(
                grePath, effectName, propCs, program);
    sEffectCreators << shaderEffectCreator;

    return shaderEffectCreator;
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
