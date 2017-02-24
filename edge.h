#ifndef EDGE_H
#define EDGE_H
#include <QPainter>
class PathPoint;
class CtrlPoint;

class Edge {
public:
    Edge(PathPoint *pt1, PathPoint *pt2, qreal pressedT);

    static void getNewRelPosForKnotInsertionAtT(QPointF P0,
                                                QPointF *P1_ptr,
                                                QPointF *P2_ptr,
                                                QPointF P3,
                                                QPointF *new_p_ptr,
                                                QPointF *new_p_start_ptr,
                                                QPointF *new_p_end_ptr,
                                                qreal t);

    static QPointF getRelPosBetweenPointsAtT(qreal t,
                                             QPointF p0Pos,
                                             QPointF p1EndPos,
                                             QPointF p2StartPos,
                                             QPointF p3Pos);

    static QPointF getRelPosBetweenPointsAtT(qreal t,
                                             PathPoint *point1,
                                             PathPoint *point2);

    void makePassThrough(QPointF absPos);

    void finishTransform();

    void startTransform();

    void setEditPath(bool bT);

    void generatePainterPath();

    void drawHover(QPainter *p);
private:
    QPainterPath mPath;

    PathPoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    PathPoint *mPoint2;
    CtrlPoint *mPoint2StartPt;
    bool mEditPath = true;

    qreal mPressedT;
};
#endif // EDGE_H
