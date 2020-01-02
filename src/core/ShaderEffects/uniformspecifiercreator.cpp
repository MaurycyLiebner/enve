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

#include "uniformspecifiercreator.h"

UniformSpecifier qrealAnimatorCreate(
        const bool glValue,
        const GLint loc,
        Property * const property,
        const qreal relFrame,
        const qreal resolution) {
    const auto qa = static_cast<QrealAnimator*>(property);
    const qreal val = qa->getEffectiveValue(relFrame)*resolution;
    const QString propName = property->prp_getName();
    const QString valScript = propName + " = " + QString::number(val);

    if(glValue) {
        Q_ASSERT(loc >= 0);
        return [loc, val, valScript](QGL33 * const gl, QJSEngine& e) {
            e.evaluate(valScript);
            gl->glUniform1f(loc, static_cast<GLfloat>(val));
        };
    } else {
        return [valScript](QGL33 * const gl, QJSEngine& e) {
            Q_UNUSED(gl)
            e.evaluate(valScript);
        };
    }
}

UniformSpecifier intAnimatorCreate(
        const bool glValue,
        const GLint loc,
        Property * const property,
        const qreal relFrame) {
    const auto ia = static_cast<IntAnimator*>(property);
    const int val = ia->getEffectiveIntValue(relFrame);
    const QString propName = property->prp_getName();
    const QString valScript = propName + " = " + QString::number(val);

    if(glValue) {
        Q_ASSERT(loc >= 0);
        return [loc, val, valScript](QGL33 * const gl, QJSEngine& e) {
            e.evaluate(valScript);
            gl->glUniform1i(loc, val);
        };
    } else {
        return [valScript](QGL33 * const gl, QJSEngine& e) {
            Q_UNUSED(gl)
            e.evaluate(valScript);
        };
    }
}

UniformSpecifier UniformSpecifierCreator::create(const GLint loc,
                                                 Property * const property,
                                                 const qreal relFrame,
                                                 const qreal resolution) const {
    if(mType == ShaderPropertyType::qrealAnimator)
        return qrealAnimatorCreate(mGLValue, loc, property, relFrame,
                                   mResolutionScaled ? resolution : 1);
    else if(mType == ShaderPropertyType::intAnimator)
        return intAnimatorCreate(mGLValue, loc, property, relFrame);
    else RuntimeThrow("Unsupported type");
}

void UniformSpecifierCreator::evaluate(QJSEngine &engine,
                                       Property * const property,
                                       const qreal relFrame,
                                       const qreal resolution) const {
    if(mType == ShaderPropertyType::qrealAnimator) {
        const auto qa = static_cast<QrealAnimator*>(property);
        qreal val = qa->getEffectiveValue(relFrame);
        if(mResolutionScaled) val *= resolution;
        engine.evaluate(property->prp_getName() + " = " + QString::number(val));
    } else if(mType == ShaderPropertyType::intAnimator) {
        const auto ia = static_cast<IntAnimator*>(property);
        const int val = ia->getEffectiveIntValue(relFrame);
        engine.evaluate(property->prp_getName() + " = " + QString::number(val));
    } else RuntimeThrow("Unsupported type");
}
