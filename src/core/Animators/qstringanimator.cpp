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

QStringAnimator::QStringAnimator(const QString &name) :
    SteppedAnimator<QString>(name) {}

QDomElement createTextElement(QDomDocument& doc,
                              const QString& text) {
    auto ele = doc.createElement("text");
    ele.appendChild(doc.createTextNode(text));
    return ele;
}

void QStringAnimator::saveSVG(QDomDocument& doc,
                              QDomElement& parent,
                              const FrameRange& absRange,
                              const qreal fps) const {
    const auto relRange = prp_absRangeToRelRange(absRange);
    const auto idRange = prp_getIdenticalRelRange(relRange.fMin);
    const int span = absRange.span();
    if(idRange.inRange(relRange) || span == 1) {
        auto ele = createTextElement(doc, getValueAtRelFrame(relRange.fMin));
        parent.appendChild(ele);
    } else {
        int i = relRange.fMin;
        const qreal div = span - 1;
        const qreal dur = div/fps;
        while(true) {
            const auto iRange = absRange*prp_getIdenticalAbsRange(i);

            const qreal begin = (iRange.fMin - absRange.fMin)/div;
            const qreal end = (iRange.fMax - absRange.fMin + 1)/div;

            auto ele = createTextElement(doc, getValueAtRelFrame(i));

            ele.setAttribute("visibility", "hidden");

            auto anim = doc.createElement("animate");
            anim.setAttribute("attributeName", "visibility");
            anim.setAttribute("values", "hidden;visible;hidden;hidden");

            QString keyTimes;
            if(!isZero6Dec(begin)) keyTimes += "0;";
            keyTimes += QString::number(begin) + ";";
            keyTimes += QString::number(end);
            if(!isOne6Dec(end)) keyTimes += ";1";
            anim.setAttribute("keyTimes", keyTimes);

            anim.setAttribute("dur", QString::number(dur) + 's');

            anim.setAttribute("repeatCount", "indefinite");

            parent.appendChild(ele);

            if(iRange.fMax >= relRange.fMax) break;
            i = prp_nextDifferentRelFrame(i);
        }
    }
}
