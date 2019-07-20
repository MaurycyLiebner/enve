#include "uniformspecifiercreator.h"

qreal stringToDouble(const QString &str) {
    if(str.isEmpty()) {
        RuntimeThrow("Can not convert an empty string to double");
    }
    bool ok;
    qreal val = str.toDouble(&ok);
    if(!ok) {
        RuntimeThrow("Can not convert '" + str + "' to double");
    }
    return val;
}

int stringToInt(const QString &str) {
    if(str.isEmpty()) RuntimeThrow("Can not convert an empty string to int");
    bool ok;
    int val = str.toInt(&ok);
    if(!ok) RuntimeThrow("Can not convert '" + str + "' to int");
    return val;
}

UniformSpecifier QrealAnimatorUniformSpecifierCreator::create(
        const GLint &loc,
        Property * const property,
        const qreal relFrame) const {
    const auto qa = GetAsPtr(property, QrealAnimator);
    const QString propName = property->prp_getName();
    const qreal val = qa->getEffectiveValue(relFrame);

    if(mScript.isEmpty()) {
        return [loc, val](QGL33 * const gl, QJSEngine&) {
            gl->glUniform1f(loc, static_cast<GLfloat>(val));
        };
    } else {
        QString script = mScript;
        return [loc, val, script, propName](
                QGL33 * const gl, QJSEngine& engine) {
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
        return [loc, val](QGL33 * const gl, QJSEngine&) {
            gl->glUniform1i(loc, val);
        };
    } else {
        QString script = mScript;
        return [loc, val, script, propName](
                QGL33 * const gl, QJSEngine& engine) {
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
