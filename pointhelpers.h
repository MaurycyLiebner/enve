#ifndef POINTHELPERS_H
#define POINTHELPERS_H
#include <QPointF>
#include <QString>
#include <QRectF>
#include <QPainter>
#include "skiaincludes.h"

enum CtrlsMode : short {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

extern qreal qRandF(qreal fMin, qreal fMax);

extern QPointF symmetricToPos(QPointF toMirror,
                              QPointF mirrorCenter);
extern QPointF symmetricToPosNewLen(QPointF toMirror,
                                    QPointF mirrorCenter,
                                    qreal newLen);
extern qreal pointToLen(QPointF point);
extern SkScalar pointToLen(SkPoint point);

extern bool isPointZero(QPointF pos);
extern QPointF scalePointToNewLen(QPointF point, qreal newLen);

extern void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                                 QPointF centerPos, QPointF *newEndPos,
                                 QPointF *newStartPos);
extern void getCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                              QPointF centerPos, QPointF *newEndPos,
                              QPointF *newStartPos);

extern qreal clamp(qreal val, qreal min, qreal max);

extern qreal qclamp(qreal val, qreal min, qreal max);

extern qreal calcCubicBezierVal(qreal p0, qreal p1,
                                qreal p2, qreal p3, qreal t);
extern QPointF calcCubicBezierVal(QPointF p0, QPointF p1,
                                  QPointF p2, QPointF p3, qreal t);

extern qreal tFromX(qreal p0x, qreal p1x,
                       qreal p2x, qreal p3x,
                       qreal x);

extern bool isZero(qreal val);

extern bool isZero2Dec(qreal val);

extern bool isZero1Dec(qreal val);

extern QString boolToSql(bool bT);

extern qreal getTforBezierPoint(const qreal &x0,
                                const qreal &x1,
                                const qreal &x2,
                                const qreal &x3,
                                const qreal &x,
                                const qreal &y0,
                                const qreal &y1,
                                const qreal &y2,
                                const qreal &y3,
                                const qreal &y,
                                qreal *error = NULL);

extern qreal getTforBezierPoint(const QPointF &p0,
                                const QPointF &p1,
                                const QPointF &p2,
                                const QPointF &p3,
                                const QPointF &p,
                                qreal *error = NULL);


extern qreal qMin4(qreal v1, qreal v2,
                   qreal v3, qreal v4);
extern qreal qMax4(qreal v1, qreal v2,
                   qreal v3, qreal v4);

extern QRectF qRectF4Points(QPointF p1, QPointF c1,
                            QPointF c2, QPointF p2);

extern QPointF rotateVector90Degrees(const QPointF &pt);

extern qreal degreesBetweenVectors(const QPointF &pt1,
                                   const QPointF &pt2);

extern qreal radiansBetweenVectors(const QPointF &pt1,
                                   const QPointF &pt2);
extern void drawCosmeticEllipse(QPainter *p,
                                const QPointF &absPos,
                                qreal rX, qreal rY);

extern qreal distBetweenTwoPoints(QPointF point1, QPointF point2);


extern bool doesPathIntersectWithCircle(const QPainterPath &path,
                                 qreal xRadius, qreal yRadius,
                                 QPointF center);

extern bool doesPathNotContainCircle(const QPainterPath &path,
                              qreal xRadius, qreal yRadius,
                              QPointF center);

extern QPointF getCenterOfPathIntersectionWithCircle(const QPainterPath &path,
                                              qreal xRadius, qreal yRadius,
                                              QPointF center);

extern QPointF getCenterOfPathDifferenceWithCircle(const QPainterPath &path,
                                            qreal xRadius, qreal yRadius,
                                            QPointF center);

extern QPointF getPointClosestOnPathTo(const QPainterPath &path,
                                       QPointF relPos,
                                       qreal xRadiusScaling,
                                       qreal yRadiusScaling);

#endif // POINTHELPERS_H
