#ifndef DRAWPATH_H
#define DRAWPATH_H
#include <QPainterPath>
#include <QPainterPathStroker>
#include "skia/skiaincludes.h"

class DrawPath {
public:
    DrawPath();

    void startDrawingPath(const QPointF &startPos,
                          const QPointF &endPos);

    void startNewSubPath(const QPointF &startPos);

    void drawPathMove(const QPointF &pos);

    void finishDrawingPath(const bool close);

    void drawPath(SkCanvas *canvas);

    void updateFittedPath();

    void addNodeAt(const QPointF &pos);

    void incNodePosIdsForIdsAboveOrEqual(const int idT,
                                         const int inc = 1);

    void decNodePosIdsForIdsBelow(const int idT,
                                  const int inc = 1);

    void sortNodes();

    void removePosForNode(const int idT);

    struct DrawPathNode {
        DrawPathNode(const int idT,
                     const QPointF &posT) {
            posId = idT;
            pos = posT;
        }
        int posId;
        QPointF pos;
    };
protected:
    QPointF mStartPos;
    QPointF mEndPos;

    QList<DrawPathNode> mNodes;
    QList<QPointF> mOriginalPoses;
    SkPath mDrawPathOriginal;
    SkPath mDrawFitted;
    //QList<QPointF> mFittedPts;
    bool mClosed = false;
};

#endif // DRAWPATH_H
