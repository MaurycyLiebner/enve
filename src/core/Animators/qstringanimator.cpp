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

QDomElement createTextElement(SvgExporter& exp, const QString& text) {
    auto textEle = exp.createElement("text");

    const QStringList lines = text.split(QRegExp("\n|\r\n|\r"));
    for(int i = 0; i < lines.count(); i++) {
        const auto& line = lines.at(i);
        auto tspan = exp.createElement("tspan");
        if(i != 0) tspan.setAttribute("dy", "1.2em");
        tspan.setAttribute("x", 0);
        const auto textNode = exp.createTextNode(line);
        tspan.appendChild(textNode);
        textEle.appendChild(tspan);
    }

    return textEle;
}

void QStringAnimator::saveSVG(SvgExporter& exp, QDomElement& parent,
                              const PropSetter& propSetter) const {
    const auto relRange = prp_absRangeToRelRange(exp.fAbsRange);
    const auto idRange = prp_getIdenticalRelRange(relRange.fMin);
    const int span = exp.fAbsRange.span();
    if(idRange.inRange(relRange) || span == 1) {
        auto ele = createTextElement(exp, getValueAtRelFrame(relRange.fMin));
        propSetter(ele);
        parent.appendChild(ele);
    } else {
        int i = relRange.fMin;
        while(true) {
            const auto iRange = exp.fAbsRange*prp_getIdenticalAbsRange(i);

            auto ele = createTextElement(exp, getValueAtRelFrame(i));
            propSetter(ele);
            SvgExportHelpers::assignVisibility(exp, ele, iRange);
            parent.appendChild(ele);

            if(iRange.fMax >= relRange.fMax) break;
            i = prp_nextDifferentRelFrame(i);
        }
    }
}

void QStringAnimator::prp_readPropertyXEV_impl(
        const QDomElement& ele, const XevImporter& imp) {
    Q_UNUSED(imp)
    readValuesXEV(ele, [](QString& str, const QStringRef& strRef) {
        str = strRef.toString();
    });
}

QDomElement QStringAnimator::prp_writePropertyXEV_impl(const XevExporter& exp) const {
    auto result = exp.createElement("Text");
    writeValuesXEV(result, [](const QString& str) { return str; });
    return result;
}
