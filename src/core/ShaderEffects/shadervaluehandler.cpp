#include "shadervaluehandler.h"

ShaderValueHandler::ShaderValueHandler(const QString &name,
                                       const bool glValue,
                                       const GLValueType type,
                                       const QString &script):
    fName(name), fGLValue(glValue), mType(type), mScript(script) {}

UniformSpecifier ShaderValueHandler::create(const GLint loc) const {
    QString name = fName;
    QString script = fName + " = " + mScript;
    if(!fGLValue) {
        return [script](QGL33 * const gl, QJSEngine& engine) {
            Q_UNUSED(gl);
            engine.evaluate(script);
        };
    }
    Q_ASSERT(loc >= 0);
    if(mType == GLValueType::Float) {
        return [loc, name, script](QGL33 * const gl, QJSEngine& engine) {
            const QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isNumber()) {
                gl->glUniform1f(loc, static_cast<GLfloat>(jsVal.toNumber()));
            } else RuntimeThrow("Invalid value script '" + script +
                                "' for '" + name + "' of type float");
        };
    } else if(mType == GLValueType::Int) {
        return [loc, name, script](QGL33 * const gl, QJSEngine& engine) {
            const QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isNumber()) {
                gl->glUniform1i(loc, static_cast<GLint>(jsVal.toInt()));
            } else RuntimeThrow("Invalid value script '" + script +
                                "' for '" + name + "' of type int");
        };
    } else if(mType == GLValueType::Vec2) {
        return [loc, name, script](QGL33 * const gl, QJSEngine& engine) {
            const QJSValue jsVal = engine.evaluate(script);
            if(jsVal.isArray()) {
                const int len = jsVal.property("length").toInt();
                if(len != 2) RuntimeThrow("Invalid value script '" + script +
                                          "' for '" + name + "' of type vec2");
                const qreal val0 = jsVal.property(0).toNumber();
                const qreal val1 = jsVal.property(1).toNumber();

                gl->glUniform2f(loc, static_cast<GLfloat>(val0),
                                static_cast<GLfloat>(val1));
            } else RuntimeThrow("Invalid value script '" + script +
                                "' for '" + name + "' of type int");
        };
    } else RuntimeThrow("Unsupported type for " + name);
}

void ShaderValueHandler::evaluate(QJSEngine &engine) const {
    engine.evaluate(fName + " = " + mScript);
}
