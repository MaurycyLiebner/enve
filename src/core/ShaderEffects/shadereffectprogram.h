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

#ifndef SHADEREFFECTPROGRAM_H
#define SHADEREFFECTPROGRAM_H
#include "uniformspecifiercreator.h"
#include "shadervaluehandler.h"

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct ShaderEffectProgram {
    GLuint fId = 0;
    GLuint fFragShader;
    GLint fGPosLoc;
    GLint fTexLocation;
    QList<GLint> fPropUniLocs;
    UniformSpecifierCreators fPropUniCreators;
    QList<stdsptr<ShaderValueHandler>> fValueHandlers;
    QList<GLint> fValueLocs;
    QString fMarginScript;

    static ShaderEffectProgram sCreateProgram(
            QGL33 * const gl, const QString &fragPath,
            const QString& marginScript,
            const QList<stdsptr<ShaderPropertyCreator>>& propCs,
            const UniformSpecifierCreators& uniCs,
            const QList<stdsptr<ShaderValueHandler>>& values);
};

#endif // SHADEREFFECTPROGRAM_H
