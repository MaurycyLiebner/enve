#include "gpurastereffectcreator.h"
#include "exceptions.h"
#include "gpurastereffect.h"
#include <QDomDocument>

QList<stdsptr<GPURasterEffectCreator>> GPURasterEffectCreator::sEffectCreators;

qsptr<Property> GPURasterEffectCreator::create() const {
    auto rasterEffect = SPtrCreate(GPURasterEffect)(&fProgram, fName);
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
            const int minVal = tryConvertingAttrToInt(elem, name, "min");
            const int maxVal = tryConvertingAttrToInt(elem, name, "max");
            const int iniVal = tryConvertingAttrToInt(elem, name, "ini");
            const int stepVal = tryConvertingAttrToInt(elem, name, "step");
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

stdsptr<GPURasterEffectCreator> GPURasterEffectCreator::sLoadFromFile(
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

    GPURasterEffectProgram program;
    try {
        program = GPURasterEffectProgram::sCreateProgram(
                    gl, fragPath, propCs, uniCs);
    } catch(...) {
        RuntimeThrow("Could not create a program for GPURasterEffect '" +
                     effectName + "'.");
    }
    auto rasterEffectCreator =
            SPtrCreate(GPURasterEffectCreator)(effectName);
    rasterEffectCreator->fProgram = program;
    rasterEffectCreator->fProperties = propCs;
    sEffectCreators << rasterEffectCreator;

    return rasterEffectCreator;
}

stdsptr<GPURasterEffectCreator> GPURasterEffectCreator::sGetCompatibleEffect(
        const QString &grePath,
        const QList<GPURasterEffectCreator::PropertyType> &props) {
    return nullptr;
}

UniformSpecifier QrealAnimatorUniformSpecifierCreator::create(
        const GLint &loc,
        Property * const property,
        const qreal relFrame) const {
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
            const QString valScript = propName + " = " + QString::number(val);
            engine.evaluate(valScript);
            const QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isNumber()) {
                gl->glUniform1f(loc, static_cast<GLfloat>(jsVal.toNumber()));
            } else if(jsVal.isArray()) {
                const QStringList vals = jsVal.toString().split(',');
                if(vals.count() == 1) {
                    const qreal val1 = stringToDouble(vals.first());
                    gl->glUniform1f(loc, static_cast<GLfloat>(val1));
                } else if(vals.count() == 2) {
                    const qreal val1 = stringToDouble(vals.first());
                    const qreal val2 = stringToDouble(vals.last());

                    gl->glUniform2f(loc, static_cast<GLfloat>(val1),
                                    static_cast<GLfloat>(val2));
                } else {
                    RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
                }
            } else {
                RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
            }
        };
    }
}

void QrealAnimatorUniformSpecifierCreator::sTestScript(const QString &script,
                                                       const QString &propName) {
    if(script.isEmpty()) return;
    QJSEngine engine;
    engine.evaluate("_texSize = [0,0]");
    engine.evaluate("_gPos = [0,0]");
    const QString valScript = propName + " = " + QString::number(0);
    engine.evaluate(valScript);
    const QJSValue jsVal = engine.evaluate(script);
    if(jsVal.isNumber()) {
    } else if(jsVal.isArray()) {
        const QStringList vals = jsVal.toString().split(',');
        if(vals.count() == 1) {
            try {
                stringToDouble(vals.first());
            } catch(...) {
                RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
            }
        } else if(vals.count() == 2) {
            try {
                stringToDouble(vals.first());
            } catch(...) {
                RuntimeThrow("Invalid glValue[0] script '" + script + "' for '" + propName + "'");
            }

            try {
                stringToDouble(vals.last());
            } catch(...) {
                RuntimeThrow("Invalid glValue[1] script '" + script + "' for '" + propName + "'");
            }
        } else {
            RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
        }
    } else {
        RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
    }
}

UniformSpecifier IntAnimatorUniformSpecifierCreator::create(
        const GLint &loc,
        Property * const property,
        const qreal relFrame) const {
    const auto ia = GetAsPtr(property, IntAnimator);
    const int val = ia->getEffectiveIntValue(relFrame);
    const QString propName = property->prp_getName();

    if(mScript.isEmpty()) {
        return [loc, val](QGL33c * const gl, QJSEngine&) {
            gl->glUniform1i(loc, val);
        };
    } else {
        QString script = mScript;
        return [loc, val, script, propName](
                QGL33c * const gl, QJSEngine& engine) {
            const QString valScript = propName + " = " + QString::number(val);
            engine.evaluate(valScript);
            const QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isNumber()) {
                gl->glUniform1i(loc, static_cast<GLint>(jsVal.toNumber()));
            } else if(jsVal.isArray()) {
                const QStringList vals = jsVal.toString().split(',');
                if(vals.count() == 1) {
                    const int val1 = stringToInt(vals.first());
                    gl->glUniform1i(loc, static_cast<GLint>(val1));
                } else if(vals.count() == 2) {
                    const int val1 = stringToInt(vals.first());
                    const int val2 = stringToInt(vals.last());

                    gl->glUniform2i(loc, static_cast<GLint>(val1),
                                    static_cast<GLint>(val2));
                } else {
                    RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
                }
            } else {
                RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
            }
        };
    }
}

void IntAnimatorUniformSpecifierCreator::sTestScript(const QString &script,
                                                     const QString &propName) {
    if(script.isEmpty()) return;
    QJSEngine engine;
    engine.evaluate("_texSize = [0,0]");
    engine.evaluate("_gPos = [0,0]");
    const QString valScript = propName + " = " + QString::number(0);
    engine.evaluate(valScript);
    const QJSValue jsVal = engine.evaluate(script);
    if(jsVal.isNumber()) {
    } else if(jsVal.isArray()) {
        const QStringList vals = jsVal.toString().split(',');
        if(vals.count() == 1) {
            try {
                stringToInt(vals.first());
            } catch(...) {
                RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
            }
        } else if(vals.count() == 2) {
            try {
                stringToInt(vals.first());
            } catch(...) {
                RuntimeThrow("Invalid glValue[0] script '" + script + "' for '" + propName + "'");
            }

            try {
                stringToInt(vals.last());
            } catch(...) {
                RuntimeThrow("Invalid glValue[1] script '" + script + "' for '" + propName + "'");
            }
        } else {
            RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
        }
    } else {
        RuntimeThrow("Invalid glValue script '" + script + "' for '" + propName + "'");
    }
}
