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

#include "svgexporthelpers.h"

QString SvgExportHelpers::ptrToStr(const void* const ptr) {
    const auto size = QT_POINTER_SIZE * 2;
    return QString("0x%1").arg((quintptr)ptr, size, 16, QChar('0'));
}

void SvgExportHelpers::assignLoop(QDomElement& ele, const bool loop) {
    if(loop) ele.setAttribute("repeatCount", "indefinite");
    else ele.setAttribute("fill", "freeze");
}
