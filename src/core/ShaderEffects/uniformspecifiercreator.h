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

#ifndef UNIFORMSPECIFIERCREATOR_H
#define UNIFORMSPECIFIERCREATOR_H
#include "qrealanimatorcreator.h"
#include "intanimatorcreator.h"
#include <QJSEngine>

enum class ShaderPropertyType {
    qrealAnimator,
    intAnimator
};

typedef std::function<void(QGL33 * const, QJSEngine&)> UniformSpecifier;
typedef QList<UniformSpecifier> UniformSpecifiers;
struct UniformSpecifierCreator : public StdSelfRef {
    UniformSpecifierCreator(const ShaderPropertyType type,
                            const bool glValue,
                            const bool resolutionScaled) :
        mType(type), mGLValue(glValue), mResolutionScaled(resolutionScaled) {}

    UniformSpecifier create(const GLint loc,
                            Property * const property,
                            const qreal relFrame,
                            const qreal resolution) const;
    void evaluate(QJSEngine& engine,
                  Property * const property,
                  const qreal relFrame,
                  const qreal resolution) const;
private:
    const ShaderPropertyType mType;
    const bool mGLValue;
    const bool mResolutionScaled;
};

#endif // UNIFORMSPECIFIERCREATOR_H
