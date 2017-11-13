#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
class Bone : public ComplexAnimator {
public:
    Bone(BasicTransformAnimator *parentAnimator) {
        mTransformAnimator = new BasicTransformAnimator();
        mTransformAnimator->setParentTransformAnimator(parentAnimator);
        mParentTransformAnimator = parentAnimator;
    }


protected:
    QPointF mRelRootPos;
    QPointF mRelTipPos;
    BasicTransformAnimator *mTransformAnimator = NULL;
    BasicTransformAnimator *mParentTransformAnimator = NULL;
    QList<Bone*> mChildBones;
};

class BonesBox : public BoundingBox {
public:
    BonesBox();
    void drawPixmapSk(SkCanvas *canvas);
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    BoundingBox *createNewDuplicate() { return NULL; }

    SkPath getCurrentPath();
    bool relPointInsidePath(const QPointF &relPos);

    bool SWT_isBonesBox() { return true; }
protected:
    Bone *mMainBone = NULL;
};

#endif // BONE_H
