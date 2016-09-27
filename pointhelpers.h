#ifndef POINTHELPERS_H
#define POINTHELPERS_H
#include <QPointF>

enum CtrlsMode {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

extern QPointF symmetricToPos(QPointF toMirror, QPointF mirrorCenter);
extern QPointF symmetricToPosNewLen(QPointF toMirror, QPointF mirrorCenter,
                                    qreal newLen);
extern qreal pointToLen(QPointF point);
extern bool isPointZero(QPointF pos);
extern QPointF scalePointToNewLen(QPointF point, qreal newLen);

extern void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                                 QPointF centerPos, QPointF *newEndPos,
                                 QPointF *newStartPos);
extern void getCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                              QPointF centerPos, QPointF *newEndPos,
                              QPointF *newStartPos);

extern qreal clamp(qreal val, qreal min, qreal max);

extern qreal calcCubicBezierVal(qreal p0, qreal p1, qreal p2, qreal p3, qreal t);

extern qreal tFromX(qreal p0x, qreal p1x, qreal p2x, qreal p3x, qreal x);

#endif // POINTHELPERS_H
