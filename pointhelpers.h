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
#endif // POINTHELPERS_H
