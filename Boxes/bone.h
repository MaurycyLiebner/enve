#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
class BonePt;
class Bone : public ComplexAnimator {
public:
    Bone(BasicTransformAnimator *parentAnimator,
         Bone *parentBone = NULL);

    Bone *getBoneAtRelPos(const QPointF &relPos);

    SkPath getCurrentRelPath();

    void drawOnCanvas(SkCanvas *canvas);

    const QPointF &getRootRelPos();
    const QPointF &getTipRelPos();

    void setRelRootPos(const QPointF &pos);

    void setRelTipPos(const QPointF &pos);

    void setAbsRootPos(const QPointF &pos);

    void setAbsTipPos(const QPointF &pos);

    BasicTransformAnimator *getTransformAnimator();

    const bool &isConnectedToParent();

    void setConnectedToParent(const bool &bT);

    Bone *getParentBone();

    BonePt *getTipPt();

    BonePt *getRootPt();
protected:
    BonePt *mTipPt = NULL;
    BonePt *mRootPt = NULL;

    bool mConnectedToParent = true;
    QPointF mRelRootPos;
    QPointF mRelTipPos;
    BasicTransformAnimator *mTransformAnimator = NULL;
    BasicTransformAnimator *mParentTransformAnimator = NULL;
    QList<Bone*> mChildBones;
    Bone *mParentBone = NULL;
};

class BonePt : public NonAnimatedMovablePoint {
public:
    BonePt(BasicTransformAnimator *parent,
           const MovablePointType &type,
           const qreal &radius = 7.5) :
        NonAnimatedMovablePoint(parent, type, radius) {
    }

    void setRelativePos(const QPointF &relPos) {
        NonAnimatedMovablePoint::setRelativePos(relPos);
        if(mTipBone != NULL) {
            if(mParentBone == mTipBone) {
                mTipBone->setRelTipPos(relPos);
            } else {
                mTipBone->setAbsTipPos(
                            mParentBone->getTransformAnimator()->
                                mapRelPosToAbs(relPos));
            }
        }
        foreach(Bone *rootBone, mRootBones) {
            if(mParentBone == rootBone) {
                rootBone->setRelRootPos(relPos);
            } else {
                rootBone->setAbsRootPos(
                            mParentBone->getTransformAnimator()->
                                mapRelPosToAbs(relPos));
            }
        }
    }

    void setTipBone(Bone *bone) {
        mTipBone = bone;
        if(bone != NULL) {
            setParentBone(bone);
        }
    }

    void addRootBone(Bone *bone) {
        mRootBones << bone;
        if(mTipBone == NULL) {
            setParentBone(bone);
        }
    }

    void setParentBone(Bone *bone) {
        mParentBone = bone;
        if(bone != NULL) {
            mParent = mParentBone->getTransformAnimator();
        }
    }
protected:
    Bone *mParentBone = NULL;
    Bone *mTipBone = NULL;
    QList<Bone*> mRootBones;
    QPointF mCurrentPos;
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

    bool relPointInsidePath(const QPointF &relPos);

    bool SWT_isBonesBox() { return true; }
protected:
    Bone *mMainBone = NULL;
};

#endif // BONE_H
