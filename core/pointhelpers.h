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
#include "Segments/qcubicsegment1d.h"
#include "Segments/qcubicsegment2d.h"
#include "Segments/quadsegment.h"
#include "Segments/cubiclist.h"

typedef std::pair<qreal, qreal> qrealPair;

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

extern void gGetCtrlsSymmetricPos(const QPointF& startPos,
                                  const QPointF& centerPos,
                                  const QPointF& endPos,
                                  QPointF &newStartPos,
                                  QPointF &newEndPos);

extern void gGetCtrlsSmoothPos(const QPointF& startPos,
                               const QPointF& centerPos,
                               const QPointF& endPos,
                               QPointF &newStartPos,
                               QPointF &newEndPos);

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

extern qCubicSegment2D gBezierLeastSquareV1V2(
        const qCubicSegment2D &seg, const QList<QPointF> &vs,
        const int &minVs, const int &maxVs);


extern void gDrawCosmeticEllipse(QPainter *p,
                                 const QPointF &absPos,
                                 const qreal &rX, const qreal &rY);


extern qreal gCubicGetTFurthestInDirection(const qCubicSegment2D& seg,
                                           const qreal &deg);

extern void gSmoothyAbsCtrlsForPtBetween(
        const QPointF &lastP, const QPointF &currP,
        const QPointF &nextP, QPointF &c1, QPointF &c2,
        qreal smoothness);
extern void gSmoothyAbsCtrlsForPtBetween(
        const SkPoint &lastP, const SkPoint &currP,
        const SkPoint &nextP, SkPoint &c1, SkPoint &c2,
        SkScalar smoothness);

extern SkPath gPathToPolyline(const SkPath& path);
extern void gForEverySegmentInPath(
        const SkPath& path,
        const std::function<void(const qCubicSegment2D&)>& func);

extern void gForEverySegmentInPath(
        const SkPath& path,
        const std::function<void(const SkPath&)>& func);

extern void gGetSmoothAbsCtrlsForPtBetween(
        const SkPoint &lastP,
        const SkPoint &currP,
        const SkPoint &nextP,
        SkPoint &c1, SkPoint &c2,
        const SkScalar &smoothLen);

extern bool gDisplaceFilterPath(SkPath* dst, const SkPath& src,
                                const SkScalar &maxDev,
                                const SkScalar &segLen,
                                const SkScalar &smoothness,
                                const uint32_t &seedAssist);

qreal gMapTToFragment(const qreal& minAbsT,
                      const qreal& maxAbsT,
                      const qreal& absT);

qreal gMapTFromFragment(const qreal& minAbsT,
                        const qreal& maxAbsT,
                        const qreal& relT);

void gGetValuesForNodeInsertion(
        const QPointF& prevP1, QPointF& prevC2,
        QPointF& newC0, QPointF& newP1, QPointF& newC2,
        QPointF &nextC0, const QPointF& nextP1,
        qreal t);

void gGetValuesForNodeRemoval(
        const QPointF &prevP1, QPointF &prevC2,
        const QPointF &pC0, const QPointF &pP1, const QPointF &pC2,
        QPointF &nextC0, const QPointF &nextP1,
        qreal t);
#endif // POINTHELPERS_H
