#ifndef EDGE_H
#define EDGE_H
#include <QPainter>
class PathPoint;
class CtrlPoint;

class Edge {
public:
    Edge(PathPoint *pt1, PathPoint *pt2, qreal pressedT);

    static void getNewRelPosForKnotInsertionAtT(const QPointF &P0,
                                                QPointF *P1_ptr,
                                                QPointF *P2_ptr,
                                                QPointF P3,
                                                QPointF *new_p_ptr,
                                                QPointF *new_p_start_ptr,
                                                QPointF *new_p_end_ptr,
                                                const qreal &t);

    static qreal getRelPosBetweenPointsAtT(const qreal &t,
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
                                             PathPoint *point1,
                                             PathPoint *point2);

    void makePassThrough(const QPointF &absPos);

    void finishPassThroughTransform();

    void startPassThroughTransform();

    void setEditPath(const bool &bT);

    void generatePainterPath();

    void drawHover(QPainter *p);

    PathPoint *getPrevPoint() const {
        return mPoint1;
    }

    PathPoint *getNextPoint() const {
        return mPoint2;
    }
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
