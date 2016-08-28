#include "pointhelpers.h"

QPointF symmetricToPos(QPointF toMirror, QPointF mirrorCenter) {
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - posDist;
}

QPointF symmetricToPosNewLen(QPointF toMirror, QPointF mirrorCenter, qreal newLen)
{
    QPointF posDist = toMirror - mirrorCenter;
    return mirrorCenter - scalePointToNewLen(posDist, newLen);
}

qreal pointToLen(QPointF point) {
    return sqrt(point.x()*point.x() + point.y()*point.y());
}

bool isPointZero(QPointF pos) {
    return abs(pos.x()) == 0 && abs(pos.y()) == 0;
}

QPointF scalePointToNewLen(QPointF point, qreal newLen) {
    if(isPointZero(point)) {
        return point;
    }
    return point * newLen / pointToLen(point);
}

void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos, QPointF centerPos,
                          QPointF *newEndPos, QPointF *newStartPos) {
    endPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(endPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    *newStartPos = (endPos*len1 + startPos*len2)/lenSum;
    *newEndPos = symmetricToPos(*newStartPos, centerPos);
}

void getCtrlsSmoothPos(QPointF endPos, QPointF startPos, QPointF centerPos,
                       QPointF *newEndPos, QPointF *newStartPos) {
    QPointF symEndPos = symmetricToPos(endPos, centerPos);
    qreal len1 = pointToLen(symEndPos);
    qreal len2 = pointToLen(startPos);
    qreal lenSum = len1 + len2;
    QPointF point1Rel = endPos - centerPos;
    QPointF point2Rel = startPos - centerPos;
    QPointF newStartDirection =
            scalePointToNewLen(
                (symEndPos*len1 + startPos*len2)/lenSum - centerPos,
                1.f);
    qreal startCtrlPtLen =
            abs(QPointF::dotProduct(point2Rel, newStartDirection));
    *newStartPos = newStartDirection*startCtrlPtLen +
            centerPos;
    qreal endCtrlPtLen =
            abs(QPointF::dotProduct(point1Rel, newStartDirection));
    *newEndPos = -newStartDirection*endCtrlPtLen +
            centerPos;
}

qreal clamp(qreal val, qreal min, qreal max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}
