#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
class BonePt;
class BonesBox;
class Bone : public ComplexAnimator {
    friend class SelfRef;
public:
    static Bone* createBone(BonesBox *boneBox);
    static Bone* createBone(Bone*  parentBone);

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

    bool isConnectedToParent();

    void setConnectedToParent(const bool &bT);

    Bone *getParentBone();

    BonePt *getTipPt();

    BonePt *getRootPt();

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const qreal &canvasScaleInv);

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    void addChildBone(const BoneQSPtr &child);

    void removeChildBone(const BoneQSPtr &child);

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePointPtr> &list);
    void setParentBone(Bone *parentBone);
    void setParentBonesBox(BonesBox *bonesBox);

    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkScalar &invScale);
    void drawHoveredOnlyThisPathSk(SkCanvas *canvas,
                                   const SkScalar &invScale);
    void deselect();
    void select();
    const bool &isSelected();

    void startPosTransform();

    void startRotTransform();

    void startScaleTransform();

    void startTransform();

    void finishTransform();
    void cancelTransform();

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

    BonesBox *getParentBox();

    void clearCurrentParent() {
        clearParentBone();
        clearParentBonesBox();
    }
protected:
    Bone();

    void clearParentBonesBox();
    void clearParentBone();

    bool mConnectedToParent = true;
    bool mSelected = false;

    QPointF mRelRootPos;
    QPointF mRelTipPos;
    QPointF mSavedTransformPivot;

    BonePtSPtr mTipPt;
    BonePtSPtr mRootPt;

    BoneTransformAnimatorQSPtr mTransformAnimator;
    QList<BoneQSPtr> mChildBones;
    BoneQPtr mParentBone;
    BonesBoxQPtr mParentBonesBox;
};

class BonePt : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
public:
    void setRelativePos(const QPointF &relPos);

    void setTipBone(Bone *bone);

    void addRootBone(Bone *bone);

    void removeRootBone(Bone *bone);

    void setParentBone(Bone *bone);

    Bone *getTipBone();

    Bone *getParentBone();
protected:
    BonePt(BasicTransformAnimator *parent);
    BoneQPtr mParentBone;
    BoneQPtr mTipBone;
    QList<BoneQPtr> mRootBones;
};

class BonesBox : public BoundingBox {
    friend class SelfRef;
public:
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
                                           QList<MovablePointPtr>& list);

    void addBone(const BoneQSPtr &bone);
    void removeBone(const BoneQSPtr &bone);
    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);
protected:
    BonesBox();

    QList<BoneQSPtr> mBones;
};

#endif // BONE_H
