#ifndef SMARTSmartVectorPath_H
#define SMARTSmartVectorPath_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"
#include "Animators/PathAnimators/smartpathcollectionhandler.h"

class NodePoint;
class BoxesGroup;
class PathAnimator;

enum CanvasMode : short;

class SmartVectorPathEdge;

class SmartVectorPath : public PathBox {
public:
    SmartVectorPath();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    void applyCurrentTransformation();
    NormalSegment getNormalSegment(const QPointF &absPos,
                                   const qreal &canvasScaleInv);

    bool SWT_isSmartVectorPath() const { return true; }
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);
    SkPath getPathAtRelFrameF(const qreal &relFrame);


    void selectAllPoints(Canvas * const canvas);
    void writeBoundingBox(QIODevice *target) {}
    void readBoundingBox(QIODevice *target) {}

    bool differenceInEditPathBetweenFrames(const int& frame1,
                                           const int& frame2) const;
    SmartPathCollectionHandler *getHandler() {
        return &mHandler;
    }
protected:
    void getMotionBlurProperties(QList<Property*> &list) const;
    SmartPathCollectionHandler mHandler;
    qptr<SmartPathCollection> mPathAnimator;
};

#endif // SMARTSmartVectorPath_H
