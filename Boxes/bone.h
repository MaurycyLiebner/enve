#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
class Bone;
class BoneTipPoint : public NonAnimatedMovablePoint {
public:
    BoneTipPoint(Bone *rootBone,
                 Bone *tipBone, Bone *parent);
    void moveByRel(const QPointF &relTranslatione);

    void moveByAbs(const QPointF &absTranslatione);
    void startTransform();
    void finishTransform();

    void setTipBone(Bone *tipBone);

    void setRootBone(Bone *rootBone);
protected:
    Bone *mTipBone = NULL;
    Bone *mRootBone = NULL;
};

class Bone : public BoundingBox {
public:
    Bone();
    void drawPixmapSk(SkCanvas *canvas);
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    NonAnimatedMovablePoint *getEndPt() { return mEndPt; }
    BoundingBox *createNewDuplicate() { return NULL; }
    void tipMovedByRel(const QPointF &trans,
                       const QPointF &savedRelPos);
    void tipMovedByAbs(const QPointF &trans,
                       const QPointF &savedRelPos);

    void rootMovedByRel(const QPointF &trans,
                        const QPointF &savedRelPos);
    void rootMovedByAbs(const QPointF &trans,
                        const QPointF &savedRelPos);

    void saveCombinedTransform() {
        mSavedCombinedTransform = getCombinedTransform();
    }

    SkPath getCurrentPath();
    bool relPointInsidePath(const QPointF &relPos);

    bool SWT_isBone() { return true; }

    void setParent(BoundingBox *parent) {
        if(parent->SWT_isBone()) {
            mRootPt = ((Bone*)parent)->getEndPt();
        } else {
            mRootPt = mThisRootPt;
        }
        BoundingBox::setParent(parent);
    }
protected:
    NonAnimatedMovablePoint *mThisRootPt = NULL;
    QMatrix mSavedCombinedTransform;
    NonAnimatedMovablePoint *mRootPt = NULL;
    BoneTipPoint *mEndPt = NULL;
};

#endif // BONE_H
