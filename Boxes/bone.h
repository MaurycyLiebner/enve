#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
class BonePt;
class BonesBox;
class Bone : public ComplexAnimator {
public:
    Bone(BonesBox *boneBox);
    Bone(Bone *parentBone);

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

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const qreal &canvasScaleInv);

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    void addChildBone(Bone *child);

    void removeChildBone(Bone *child);

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    void setParentBone(Bone *parentBone);
    void setParentBonesBox(BonesBox *bonesBox);

    void drawHoveredPathSk(SkCanvas *canvas,
                           const qreal &invScale);
    void drawHoveredOnlyThisPathSk(SkCanvas *canvas,
                                   const qreal &invScale);
    void deselect();
    void select();
    const bool &isSelected();

    void startPosTransform();

    void startRotTransform();

    void startScaleTransform();

    void startTransform();

    void finishTransform();
    void cancelTransform() {
        mTransformAnimator->prp_cancelTransform();
    }

    void moveByAbs(const QPointF &trans);

    void moveByRel(const QPointF &trans);

    void setAbsolutePos(const QPointF &pos,
                                     const bool &saveUndoRedo);

    void setRelativePos(const QPointF &relPos,
                                     const bool &saveUndoRedo);

    void saveTransformPivotAbsPos(const QPointF &absPivot);

    QPointF mapAbsPosToRel(const QPointF &absPos);

    QPointF mapRelPosToAbs(const QPointF &relPos);

    void scale(const qreal &scaleBy);

    void scale(const qreal &scaleXBy, const qreal &scaleYBy);

    void rotateBy(const qreal &rot);

    void rotateRelativeToSavedPivot(const qreal &rot);

    void scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                   const qreal &scaleYBy);

    BonesBox *getParentBox() {
        if(mParentBonesBox == NULL) {
            return mParentBone->getParentBox();
        }
        return mParentBonesBox;
    }
protected:
    QPointF mSavedTransformPivot;
    bool mSelected = false;
    BonePt *mTipPt = NULL;
    BonePt *mRootPt = NULL;

    bool mConnectedToParent = true;
    QPointF mRelRootPos;
    QPointF mRelTipPos;
    QSharedPointer<BoneTransformAnimator> mTransformAnimator;
    QList<Bone*> mChildBones;
    Bone *mParentBone = NULL;
    BonesBox *mParentBonesBox = NULL;
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

    void removeRootBone(Bone *bone) {
        mRootBones.removeOne(bone);
    }

    void setParentBone(Bone *bone) {
        mParentBone = bone;
        if(bone != NULL) {
            mParent = mParentBone->getTransformAnimator();
        }
    }

    Bone *getTipBone() {
        return mTipBone;
    }

    Bone *getParentBone() {
        return mParentBone;
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

    BoundingBox *createNewDuplicate();

    Bone *getBoneAtAbsPos(const QPointF &absPos);

    Bone *getBoneAtRelPos(const QPointF &relPos);

    bool relPointInsidePath(const QPointF &relPos);

    bool SWT_isBonesBox();

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);

    void addBone(Bone *bone);

    void removeBone(Bone *bone);
    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);
protected:
    QList<Bone*> mBones;
};

#endif // BONE_H
