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

#ifndef SVGEXPORTHELPERS_H
#define SVGEXPORTHELPERS_H

#include <QtCore>
#include <QDomElement>

#include "include/core/SkImage.h"

#include "framerange.h"

namespace SvgExportHelpers {
    QString ptrToStr(const void* const ptr);
    void assignLoop(QDomElement& ele, const bool loop);
    void defImage(QDomDocument& doc, QDomElement& defs,
                  const sk_sp<SkImage>& image,
                  const QString id);
    void assignVisibility(QDomDocument& doc,
                          QDomElement& ele,
                          const FrameRange& visRange,
                          const FrameRange& absRange,
                          const qreal fps, const bool loop);
};

#endif // SVGEXPORTHELPERS_H
