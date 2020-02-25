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
    const auto anim = static_cast<QrealAnimator*>(property);
    const qreal val = anim->getEffectiveValue(relFrame)*resolution;
    const QString propName = anim->prp_getName();
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
        const qreal relFrame,
        const qreal resolution) {
    const auto anim = static_cast<IntAnimator*>(property);
    const int val = qRound(anim->getEffectiveIntValue(relFrame)*resolution);
    const QString valScript = anim->prp_getName() + " = " + QString::number(val);

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

QString vec2ValScript(const QString& name, const QPointF& value) {
    return name + " = [" + QString::number(value.x()) + "," +
                           QString::number(value.y()) + "]";
}

UniformSpecifier qPointFAnimatorCreate(
        const bool glValue,
        const GLint loc,
        Property * const property,
        const qreal relFrame,
        const qreal resolution) {
    const auto anim = static_cast<QPointFAnimator*>(property);
    const QPointF val = anim->getEffectiveValue(relFrame)*resolution;
    const QString valScript = vec2ValScript(anim->prp_getName(), val);

    if(glValue) {
        Q_ASSERT(loc >= 0);
        return [loc, val, valScript](QGL33 * const gl, QJSEngine& e) {
            e.evaluate(valScript);
            gl->glUniform2f(loc, val.x(), val.y());
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
    switch(mType) {
    case ShaderPropertyType::floatProperty:
        return qrealAnimatorCreate(mGLValue, loc, property, relFrame,
                                   mResolutionScaled ? resolution : 1);
    case ShaderPropertyType::intProperty:
        return intAnimatorCreate(mGLValue, loc, property, relFrame,
                                 mResolutionScaled ? resolution : 1);
    case ShaderPropertyType::vec2Property:
        return qPointFAnimatorCreate(mGLValue, loc, property, relFrame,
                                     mResolutionScaled ? resolution : 1);
    default: RuntimeThrow("Unsupported type");
    }
}

void UniformSpecifierCreator::evaluate(QJSEngine &engine,
                                       Property * const property,
                                       const qreal relFrame,
                                       const qreal resolution) const {
    switch(mType) {
    case ShaderPropertyType::floatProperty: {
        const auto anim = static_cast<QrealAnimator*>(property);
        qreal val = anim->getEffectiveValue(relFrame);
        if(mResolutionScaled) val *= resolution;
        engine.evaluate(anim->prp_getName() + " = " + QString::number(val));
    } break;
    case ShaderPropertyType::intProperty: {
        const auto anim = static_cast<IntAnimator*>(property);
        int val = anim->getEffectiveIntValue(relFrame);
        if(mResolutionScaled) val = qRound(val*resolution);
        engine.evaluate(anim->prp_getName() + " = " + QString::number(val));
    } break;
    case ShaderPropertyType::vec2Property: {
        const auto anim = static_cast<QPointFAnimator*>(property);
        QPointF val = anim->getEffectiveValue(relFrame);
        if(mResolutionScaled) val *= resolution;
        engine.evaluate(vec2ValScript(anim->prp_getName(), val));
    } break;
    default: RuntimeThrow("Unsupported type");
    }
}
