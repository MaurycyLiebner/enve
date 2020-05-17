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

#ifndef XMLEXPORTHELPERS_H
#define XMLEXPORTHELPERS_H

#include "../exceptions.h"

#include "../skia/skiaincludes.h"

#include <QDomElement>
#include <QString>

class SimpleBrushWrapper;

namespace XmlExportHelpers {
    CORE_EXPORT
    SkBlendMode stringToBlendMode(const QString& compOpStr);
    CORE_EXPORT
    QString blendModeToString(const SkBlendMode blendMode);

    CORE_EXPORT
    qreal stringToDouble(const QStringRef& string);
    CORE_EXPORT
    qreal stringToDouble(const QString& string);
    CORE_EXPORT
    int stringToInt(const QStringRef& string);
    CORE_EXPORT
    int stringToInt(const QString& string);

    template <typename T, typename S>
    T stringToEnum(const S& string) {
        const int intVal = stringToInt(string);
        return static_cast<T>(intVal);
    }

    template <typename T, typename S>
    T stringToEnum(const S& string, const T min, const T max) {
        const auto result = stringToEnum<T>(string);
        if(result < min || result > max)
            RuntimeThrow("Value outside of enum value range");
        return result;
    }

    template <typename T, typename S>
    T stringToEnum(const S& string, const T max) {
        return stringToEnum(string, 0, max);
    }

    CORE_EXPORT
    QMatrix stringToMatrix(const QString& str);
    CORE_EXPORT
    QString matrixToString(const QMatrix& m);
};

namespace XevExportHelpers {
    CORE_EXPORT
    QDomElement brushToElement(SimpleBrushWrapper* const brush,
                               QDomDocument& doc);
    CORE_EXPORT
    SimpleBrushWrapper* brushFromElement(const QDomElement& ele);
};

#endif // XMLEXPORTHELPERS_H