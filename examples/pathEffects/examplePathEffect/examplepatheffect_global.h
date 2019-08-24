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

#ifndef EXAMPLEPATHEFFECT_GLOBAL_H
#define EXAMPLEPATHEFFECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXAMPLEPATHEFFECT_LIBRARY)
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXAMPLEPATHEFFECTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "enveCore/PathEffects/custompatheffect.h"

extern "C" {

EXAMPLEPATHEFFECTSHARED_EXPORT
    qsptr<CustomPathEffect> eCreateNewestVersion();

EXAMPLEPATHEFFECTSHARED_EXPORT
    qsptr<CustomPathEffect> eCreate(
        const CustomIdentifier &identifier);

EXAMPLEPATHEFFECTSHARED_EXPORT
    QString eName();

EXAMPLEPATHEFFECTSHARED_EXPORT
    CustomIdentifier eIdentifier();

EXAMPLEPATHEFFECTSHARED_EXPORT
    bool eSupports(const CustomIdentifier &identifier);

}
#endif // EXAMPLEPATHEFFECT_GLOBAL_H
