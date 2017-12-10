#ifndef DRAWPATH_H
#define DRAWPATH_H
#include <QPainterPath>
#include <QPainterPathStroker>
#include "skiaincludes.h"

class DrawPath {
public:
    DrawPath();

    void startDrawingPath(const QPointF &startPos,
                          const QPointF &endPos);

    void startNewSubPath(const QPointF &startPos);

    void drawPathMove(const QPointF &pos);

    void finishDrawingPath(const bool &close);

    void drawPath(SkCanvas *canvas);

    void updateFittingPts();

    void addNodeAt(const QPointF &pos);

    void incNodePosIdsForIdsAboveOrEqual(const int &idT,
                                         const int &inc = 1);

    void decNodePosIdsForIdsBelow(const int &idT,
                                  const int &inc = 1);

    void sortNodes();

    void removePosForNode(const int &idT);
protected:
    QPointF mStartPos;
    QPointF mEndPos;

    QList<int> mNodePosIds;
    QList<QPointF> mOriginalPoses;
    QList<QPointF> mFittingPts;
    SkPath mDrawPathOriginal;
    SkPath mDrawFitted;
    bool mClosed = false;
};

#endif // DRAWPATH_H
