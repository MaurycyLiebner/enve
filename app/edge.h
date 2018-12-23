#ifndef EDGE_H
#define EDGE_H
#include <QPainter>
#include "smartPointers/sharedpointerdefs.h"
class NodePoint;
class CtrlPoint;
#include "skia/skiaincludes.h"

class VectorPathEdge : public StdSelfRef {
public:
    VectorPathEdge(NodePoint *pt1, NodePoint *pt2);

    static void getNewRelPosForKnotInsertionAtT(const QPointF &P0,
                                                QPointF *P1_ptr,
                                                QPointF *P2_ptr,
                                                const QPointF &P3,
                                                QPointF *new_p_ptr,
                                                QPointF *new_p_start_ptr,
                                                QPointF *new_p_end_ptr,
                                                const qreal &t);

    static void getNewRelPosForKnotInsertionAtTSk(const SkPoint &P0,
                                                  SkPoint *P1_ptr,
                                                  SkPoint *P2_ptr,
                                                  SkPoint P3,
                                                  SkPoint *new_p_ptr,
                                                  SkPoint *new_p_start_ptr,
                                                  SkPoint *new_p_end_ptr,
                                                  const SkScalar &t);

    static QPointF getPosBetweenPointsAtT(const qreal &t,
                                          const QPointF &p0Pos,
                                          const QPointF &p1EndPos,
                                          const QPointF &p2StartPos,
                                          const QPointF &p3Pos);

    static qreal getLength(const QPointF &p0Pos,
                           const QPointF &p1EndPos,
                           const QPointF &p2StartPos,
                           const QPointF &p3Pos,
                           int divisions);

    static qreal getLength(const QPointF &p0Pos,
                           const QPointF &p1EndPos,
                           const QPointF &p2StartPos,
                           const QPointF &p3Pos);

    static QPointF getRelPosBetweenPointsAtT(const qreal &t,
                                             NodePoint *point1,
                                             NodePoint *point2);
    static QPointF getAbsPosBetweenPointsAtT(const qreal &t,
                                             NodePoint *point1,
                                             NodePoint *point2);

    QPointF getRelPosAtT(const qreal &t);
    QPointF getAbsPosAtT(const qreal &t);

    void makePassThrough(const QPointF &absPos);

    void finishPassThroughTransform();
    void startPassThroughTransform();
    void cancelPassThroughTransform();

    void setEditPath(const bool &bT);

    void generatePainterPath();

    void drawHoveredSk(SkCanvas *canvas,
                       const SkScalar &invScale);

    NodePoint *getPoint1() const;

    NodePoint *getPoint2() const;

    void setPoint1(NodePoint *point1);

    void setPoint2(NodePoint *point2);

    void setPressedT(const qreal &t);

    void getNearestAbsPosAndT(const QPointF &absPos,
                              QPointF *nearestPoint,
                              qreal *t);

    QPointF getSlopeVector(const qreal &t);
    void getNearestRelPosAndT(const QPointF &relPos,
                              QPointF *nearestPoint,
                              qreal *t, qreal *error);
private:
    SkPath mSkPath;

    NodePoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    NodePoint *mPoint2;
    CtrlPoint *mPoint2StartPt;
    bool mEditPath = true;

    qreal mPressedT;
};

#endif // EDGE_H
