// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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
            Q_UNUSED(gl)
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
