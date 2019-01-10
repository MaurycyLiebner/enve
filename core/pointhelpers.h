#ifndef POINTHELPERS_H
#define POINTHELPERS_H
#include <QPointF>
#include <QString>
#include <QRectF>
#include <QPainter>
#include <type_traits>
#include <experimental/type_traits>
#include "simplemath.h"
#include "Segments/conicsegment.h"
#include "Segments/cubicsegment.h"
#include "Segments/quadsegment.h"

typedef std::pair<qCubicSegment2D, qCubicSegment2D> CubicPair;
typedef std::pair<qreal, qreal> qrealPair;
typedef QList<qCubicSegment2D> CubicList;
typedef std::pair<CubicList, CubicList> CubicListPair;
#define pow2(val) val*val
#define pow3(val) val*val*val

enum CtrlsMode : short {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

//template <typename T>
//struct Solution {
//    Solution(const bool& valid, const T& value = T()) :
//        fValid(valid), fValue(value) {}

//    operator const bool&() const {
//        return fValid;
//    }

//    operator const T&() const {
//        return fValue;
//    }

//    const bool fValid;
//    const T fValue;
//};

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

extern void gGetCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                                  QPointF centerPos, QPointF *newEndPos,
                                  QPointF *newStartPos);
extern void gGetCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                               QPointF centerPos, QPointF *newEndPos,
                               QPointF *newStartPos);

extern qreal gCalcCubicBezierVal(const qCubicSegment1D &seg,
                                 const qreal &t);
extern QPointF gCalcCubicBezierVal(const qCubicSegment2D &seg,
                                   const qreal& t);

extern qreal gTFromX(const qCubicSegment1D &seg,
                    const qreal &x);


extern QPointF gGetClosestPointOnLineSegment(const QPointF &a,
                                             const QPointF &b,
                                             const QPointF &p);
extern QPointF gClosestPointOnRect(const QRectF &rect,
                                   const QPointF &point,
                                   qreal *dist = nullptr);

extern qreal gGetTforBezierPoint(const qCubicSegment2D &seg,
                                 const QPointF &pos,
                                 qreal *error = nullptr,
                                 QPointF *bestPosPtr = nullptr,
                                 const bool &fineTune = true);

extern qreal gGetClosestTValueOnBezier(const qCubicSegment2D &seg,
                                       const QPointF &p,
                                       QPointF *bestPosPtr = nullptr,
                                       qreal *errorPtr = nullptr);

extern qreal gGetBezierTValueForX(const qCubicSegment1D &seg,
                                  const qreal &x,
                                  qreal *error = nullptr);
extern qCubicSegment2D gBezierLeastSquareV1V2(
        const qCubicSegment2D &seg, const QList<QPointF> &vs,
        const int &minVs, const int &maxVs);


extern void gDrawCosmeticEllipse(QPainter *p,
                                 const QPointF &absPos,
                                 qreal rX, qreal rY);

extern qreal gDistBetweenTwoPoints(QPointF point1, QPointF point2);


extern bool gDoesPathIntersectWithCircle(const QPainterPath &path,
                                  qreal xRadius, qreal yRadius,
                                  QPointF center);

extern bool gDoesPathNotContainCircle(const QPainterPath &path,
                              qreal xRadius, qreal yRadius,
                              QPointF center);

extern QPointF gGetCenterOfPathIntersectionWithCircle(const QPainterPath &path,
                                              qreal xRadius, qreal yRadius,
                                              QPointF center);

extern QPointF gGetCenterOfPathDifferenceWithCircle(const QPainterPath &path,
                                            qreal xRadius, qreal yRadius,
                                            QPointF center);

extern QPointF gGetPointClosestOnPathTo(const QPainterPath &path,
                                       QPointF relPos,
                                       qreal xRadiusScaling,
                                       qreal yRadiusScaling);

extern qreal gGetBezierTValueForXAssumeNoOverlapGrowingOnly(
        const qCubicSegment1D &seg,
        const qreal &x,
        const qreal &maxError, qreal *error = nullptr);

extern qreal gMinDistanceToPath(const SkPoint& pos, const SkPath& path);

extern QList<qCubicSegment2D> gPathToQCubicSegs2D(const SkPath& path);


extern void gSegmentLengthLoop(const qCubicSegment2D& seg,
                               const uint &div,
                               const bool& closedInterval,
                               const std::function<void(const qreal&)> &func);


//! @brief Splits segments at intersections.
//! Returns pair of lists of segments corresponding to each input segment.
extern CubicListPair gSplitAtIntersections(const qCubicSegment2D& seg1,
                                           const qCubicSegment2D& seg2);
extern SkPath gCubicListToSkPath(const CubicList& list);

extern CubicList gSplitSelfAtIntersections(const qCubicSegment2D& seg);

extern CubicList gRemoveAllPointsCloserThan(const qreal& minDist,
                                            const SkPath& distTo,
                                            const CubicList &src);

extern CubicList gCubicIntersectList(CubicList targetList);

extern CubicList gCubicIntersectListAndRemoveExt(CubicList targetList);

extern CubicList gRemoveAllPointsOutsidePath(const SkPath& path,
                                             const CubicList &src);
extern CubicList gRemoveAllPointsInsidePath(const SkPath& path,
                                            const CubicList &src);

extern qreal gCubicLength(const qCubicSegment2D& seg);
extern qreal gCubicTimeAtLength(const qCubicSegment2D& seg,
                                const qreal& length);
#endif // POINTHELPERS_H
