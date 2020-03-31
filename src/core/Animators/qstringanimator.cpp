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

#include "qstringanimator.h"

#include "undoredo.h"
#include "simplemath.h"
#include "svgexporthelpers.h"

QStringAnimator::QStringAnimator(const QString &name) :
    SteppedAnimator<QString>(name) {}

QDomElement createTextElement(SvgExporter& exp,
                              const QString& text) {
    auto ele = exp.createElement("text");
    ele.appendChild(exp.createTextNode(text));
    return ele;
}

void QStringAnimator::saveSVG(SvgExporter& exp,
                              QDomElement& parent) const {
    const auto relRange = prp_absRangeToRelRange(exp.fAbsRange);
    const auto idRange = prp_getIdenticalRelRange(relRange.fMin);
    const int span = exp.fAbsRange.span();
    if(idRange.inRange(relRange) || span == 1) {
        auto ele = createTextElement(exp, getValueAtRelFrame(relRange.fMin));
        parent.appendChild(ele);
    } else {
        int i = relRange.fMin;
        while(true) {
            const auto iRange = exp.fAbsRange*prp_getIdenticalAbsRange(i);

            auto ele = createTextElement(exp, getValueAtRelFrame(i));
            SvgExportHelpers::assignVisibility(exp, ele, iRange);
            parent.appendChild(ele);

            if(iRange.fMax >= relRange.fMax) break;
            i = prp_nextDifferentRelFrame(i);
        }
    }
}
