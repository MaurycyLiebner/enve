#include "linespatheffect.h"
#include "Animators/qrealanimator.h"

LinesPathEffect::LinesPathEffect(const bool outlinePathEffect) :
    PathEffect("lines effect", LINES_PATH_EFFECT, outlinePathEffect) {
    mAngle = SPtrCreate(QrealAnimator)(0, -9999, 9999, 1, "angle");
    mDistance = SPtrCreate(QrealAnimator)(10, 1, 9999, 1, "distance");

    ca_addChildAnimator(mAngle);
    ca_addChildAnimator(mDistance);
}

void LinesPathEffect::apply(const qreal relFrame, const SkPath &src,
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
            dst->moveTo(toSkPoint(line.p1()));
            dst->lineTo(toSkPoint(line.p2()));
        }
    }
}

void LinesPathEffect::writeProperty(QIODevice * const dst) const {
    PathEffect::writeProperty(dst);
    mAngle->writeProperty(dst);
    mDistance->writeProperty(dst);
}

void LinesPathEffect::readProperty(QIODevice * const src) {
    PathEffect::readProperty(src);
    mAngle->readProperty(src);
    mDistance->readProperty(src);
}
