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

extern qreal gSolveForP1(const qreal& p0, const qreal& p2,
                         const qreal& p3, const qreal& t,
                         const qreal& value);

extern qreal gSolveForP2(const qreal& p0, const qreal& p1,
                         const qreal& p3, const qreal& t,
                         const qreal& value);

extern void gGetCtrlsSymmetricPos(const QPointF& endPos,
                                  const QPointF& startPos,
                                  const QPointF& centerPos,
                                  QPointF &newEndPos,
                                  QPointF &newStartPos);

extern void gGetCtrlsSmoothPos(const QPointF &endPos,
                               const QPointF &startPos,
                               const QPointF &centerPos,
                               QPointF &newEndPos,
                               QPointF &newStartPos);

extern qreal gCubicValueAtT(const qCubicSegment1D &seg,
                                 const qreal &t);
extern QPointF gCubicValueAtT(const qCubicSegment2D &seg,
                                   const qreal& t);

extern qreal gTFromX(const qCubicSegment1D &seg,
                    const qreal &x);


extern QPointF gGetClosestPointOnLineSegment(const QPointF &a,
                                             const QPointF &b,
                                             const QPointF &p);
extern QPointF gClosestPointOnRect(const QRectF &rect,
                                   const QPointF &point,
                                   qreal *dist = nullptr);

extern qreal gGetClosestTValueOnBezier(const qCubicSegment2D &seg,
                                       const QPointF &p,
                                       const qreal &minT,
                                       const qreal &maxT,
                                       QPointF *bestPosPtr = nullptr,
                                       qreal *errorPtr = nullptr);

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

extern qreal gGetBezierTValueForXAssumeNoOverlapGrowingOnly(
        const qCubicSegment1D &seg,
        const qreal &x,
        const qreal &maxError, qreal *error = nullptr);

extern qreal gMinDistanceToPath(const SkPoint& pos, const SkPath& path);

extern QList<qCubicSegment2D> gPathToQCubicSegs2D(const SkPath& path);


//! @brief Splits segments at intersections.
//! Returns pair of lists of segments corresponding to each input segment.
extern CubicListPair gSplitAtIntersections(const qCubicSegment2D& seg1,
                                           const qCubicSegment2D& seg2);
extern SkPath gCubicListToSkPath(const CubicList& list);

extern CubicList gSplitSelfAtIntersections(const qCubicSegment2D& seg);

extern CubicList gCubicIntersectList(CubicList targetList);

extern qreal gCubicLength(const qCubicSegment2D& seg);
extern qreal gCubicTimeAtLength(const qCubicSegment2D& seg,
                                const qreal& length);
extern qreal gCubicLengthAtT(const qCubicSegment2D& seg, qreal t);

extern qreal gCubicGetTFurthestInDirection(const qCubicSegment2D& seg,
                                           const qreal &deg);
extern bool gCubicListClockWise(const CubicList &list);

extern QList<SkPath> gSolidifyCubicList(const CubicList& list,
                                        const qreal &width);
extern SkPath gCubicToSkPath(const qCubicSegment2D& seg);
extern void gGetSmoothAbsCtrlsForPtBetween(const SkPoint &lastP,
                                        const SkPoint &currP,
                                        const SkPoint &nextP,
                                        SkPoint &c1, SkPoint &c2,
                                        const SkScalar &smoothLen);

extern SkPath gSmoothyPath(const SkPath& path,
                           const SkScalar& smootness);

extern QList<CubicList> gPathToQCubicSegs2DBreakApart(const SkPath& path);

extern qCubicSegment2D gCubicRandomDisplace(const qCubicSegment2D& seg,
                                            const qreal& displ);
#endif // POINTHELPERS_H
