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

#include "zigzagpatheffect.h"
#include "Animators/qrealanimator.h"

ZigZagPathEffect::ZigZagPathEffect() :
    PathEffect("zigzag effect", PathEffectType::ZIGZAG) {
    mAngle = enve::make_shared<QrealAnimator>(0, -9999, 9999, 1, "angle");
    mDistance = enve::make_shared<QrealAnimator>(10, 1, 9999, 1, "distance");

    ca_addChild(mAngle);
    ca_addChild(mDistance);
}

void ZigZagPathEffect::apply(const qreal relFrame, const SkPath &src,
                             SkPath * const dst) {
    const qreal degAngle = mAngle->getEffectiveValue(relFrame);
    const qreal distInc = mDistance->getEffectiveValue(relFrame);

    dst->reset();
    auto segLists = CubicList::sMakeFromSkPath(src);
    const QRectF pathBounds = toQRectF(src.getBounds());
    QTransform rotate;
    const QPointF pivot = pathBounds.center();
    rotate.translate(pivot.x(), pivot.y());
    rotate.rotate(degAngle);
    rotate.translate(-pivot.x(), -pivot.y());
    const QPolygonF linesBBPolygon = rotate.map(QPolygonF(pathBounds));
    const QRectF secondBB = linesBBPolygon.boundingRect();
    const QPolygonF secondLinesBB = rotate.map(QPolygonF(secondBB));
    const QLineF firstLine(secondLinesBB.at(0), secondLinesBB.at(1));
    const QLineF sideLine(secondLinesBB.at(1), secondLinesBB.at(2));
    const int nLines = qCeil(sideLine.length()/distInc);
//    const QLineF transVec = QLineF::fromPolar(distInc, degAngle - 90);
    QLineF transVec = sideLine;
    transVec.setLength(distInc);
    const QPointF transPt(transVec.dx(), transVec.dy());
    QList<QLineF> prevLines;
    for(int i = 0; i < nLines; i++) {
        const QLineF iLine = firstLine.translated(i*transPt);
        QList<QPointF> intersections;
        for(auto& seg : segLists) {
//            intersections.append(iLine.p1());
//            intersections.append(iLine.p2());
            seg.lineIntersections(iLine, intersections);
        }

        const int jMin = 0;
        const int jMax = intersections.count() - 2;
        QList<QLineF> currLines;
        for(int j = jMin; j <= jMax; j += 2) {
            const QLineF line(intersections.at(j), intersections.at(j + 1));
            currLines << line;
        }

        if(prevLines.count() == 1 && currLines.count() == 1) {
            const auto& currLine = currLines.first();
            dst->lineTo(toSkPoint(currLine.p1()));
            dst->lineTo(toSkPoint(currLine.p2()));
        } else {
            for(const auto& line : currLines) {
                dst->moveTo(toSkPoint(line.p1()));
                dst->lineTo(toSkPoint(line.p2()));
            }
        }

        prevLines = currLines;
    }
}
