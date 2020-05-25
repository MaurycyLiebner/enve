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

#include "shadereffectprogram.h"

std::unique_ptr<ShaderEffectProgram>
ShaderEffectProgram::sCreateProgram(
        QGL33 * const gl, const QString &fragPath,
        const QList<stdsptr<ShaderPropertyCreator>>& propCs,
        const std::shared_ptr<ShaderEffectJS::Blueprint>& jsBlueprint,
        const UniformSpecifierCreators& uniCs,
        const QList<stdsptr<ShaderValueHandler>>& values) {
    std::unique_ptr<ShaderEffectProgram> program =
            std::make_unique<ShaderEffectProgram>();
    try {
        gIniProgram(gl, program->fId, GL_TEXTURED_VERT, fragPath);
    } catch(...) {
        RuntimeThrow("Could not initialize a program for ShaderEffectProgram");
    }

    program->fJSBlueprint = jsBlueprint;

    for(const auto& propC : propCs) {
        if(propC->fGLValue) {
            const GLint loc = gl->glGetUniformLocation(program->fId,
                                                       propC->fName.toLatin1());
            if(loc < 0) {
                gl->glDeleteProgram(program->fId);
                RuntimeThrow("'" + propC->fName +
                             "' does not correspond to an active uniform variable.");
            }
            program->fPropUniLocs.append(loc);
        } else program->fPropUniLocs.append(-1);
    }
    program->fPropUniCreators = uniCs;
    for(const auto& value : values) {
        const GLint loc = gl->glGetUniformLocation(program->fId,
                                                   value->fName.toLatin1());
        if(loc < 0) {
            gl->glDeleteProgram(program->fId);
            RuntimeThrow("'" + value->fName +
                         "' does not correspond to an active uniform variable.");
        }
        program->fValueLocs.append(loc);
    }
    program->fValueHandlers = values;

    program->fTexLocation = gl->glGetUniformLocation(program->fId, "texture");
    if(program->fTexLocation < 0) {
        gl->glDeleteProgram(program->fId);
        RuntimeThrow("Invalid location received for 'texture'.");
    }
    gl->glUniform1i(program->fTexLocation, 0);

    return program;
}
