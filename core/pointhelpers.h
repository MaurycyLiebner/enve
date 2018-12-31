#ifndef POINTHELPERS_H
#define POINTHELPERS_H
#include <QPointF>
#include <QString>
#include <QRectF>
#include <QPainter>
#include "simplemath.h"
enum CtrlsMode : short {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};
extern qreal gSolveForP2(const qreal& p0, const qreal& p1,
                        const qreal& p3, const qreal& t,
                          const qreal& value);

extern qreal gSolveForP1(const qreal& p0, const qreal& p2,
                         const qreal& p3, const qreal& t,
                         const qreal& value);

extern qreal gSolveForP2(const qreal& p0, const qreal& p1,
                        const qreal& p3, const qreal& t,
                          const qreal& value);

extern qreal gSolveForP1(const qreal& p0, const qreal& p2,
                         const qreal& p3, const qreal& t,
                         const qreal& value);

extern void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                                 QPointF centerPos, QPointF *newEndPos,
                                 QPointF *newStartPos);
extern void getCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                              QPointF centerPos, QPointF *newEndPos,
                              QPointF *newStartPos);

extern qreal calcCubicBezierVal(qreal p0, qreal p1,
                                qreal p2, qreal p3, qreal t);
extern QPointF calcCubicBezierVal(QPointF p0, QPointF p1,
                                  QPointF p2, QPointF p3, qreal t);

extern qreal tFromX(qreal p0x, qreal p1x,
                       qreal p2x, qreal p3x,
                       qreal x);


extern QPointF getClosestPointOnLineSegment(const QPointF &a,
                                            const QPointF &b,
                                            const QPointF &p);
extern QPointF closestPointOnRect(const QRectF &rect,
                                  const QPointF &point,
                                  qreal *dist = nullptr);

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
                                qreal *error = nullptr,
                                QPointF *bestPosPtr = nullptr,
                                const bool &fineTune = true);

extern qreal getTforBezierPoint(const QPointF &p0,
                                const QPointF &p1,
                                const QPointF &p2,
                                const QPointF &p3,
                                const QPointF &p,
                                qreal *error = nullptr,
                                QPointF *bestPosPtr = nullptr);

extern void getClosestTValuesBezier1D(const qreal &v0n,
                                      const qreal &v1n,
                                      const qreal &v2n,
                                      const qreal &v3n,
                                      const qreal &vn,
                                      QList<qreal> *list);

extern qreal getClosestTValueBezier2D(const QPointF &p0,
                                      const QPointF &p1,
                                      const QPointF &p2,
                                      const QPointF &p3,
                                      const QPointF &p,
                                      QPointF *bestPosPtr = nullptr,
                                      qreal *errorPtr = nullptr);

extern qreal getBezierTValueForX(const qreal &x0,
                                 const qreal &x1,
                                 const qreal &x2,
                                 const qreal &x3,
                                 const qreal &x,
                                 qreal *error = nullptr);
extern void bezierLeastSquareV1V2(const QPointF &v0,
                                  QPointF &v1, QPointF &v2,
                                  const QPointF &v3,
                                  const QList<QPointF> &vs,
                                  const int &minVs,
                                  const int &maxVs);


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

extern qreal getBezierTValueForXAssumeNoOverlapGrowingOnly(const qreal &x0,
                                                           const qreal &x1,
                                                           const qreal &x2,
                                                           const qreal &x3,
                                                           const qreal &x,
                                                           const qreal &maxError,
                                                           qreal *error = nullptr);

#endif // POINTHELPERS_H
