#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"
#include "MovablePoints/nonanimatedmovablepoint.h"
#include "Animators/transformanimator.h"
class BonePt;
class BonesBox;
class Bone : public ComplexAnimator {
    friend class SelfRef;
public:
    static Bone* createBone(BonesBox *boneBox);
    static Bone* createBone(Bone*  parentBone);

    Bone *getBoneAtRelPos(const QPointF &relPos);

    SkPath getCurrentRelPath();

    void drawOnCanvas(SkCanvas * const canvas);

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

    void drawOnCanvas(SkCanvas * const canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale);

    void addChildBone(const qsptr<Bone> &child);

    void removeChildBone(const qsptr<Bone> &child);

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);
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

    void setAbsolutePos(const QPointF &pos);

    void setRelativePos(const QPointF &relPos);

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

    stdsptr<BonePt> mTipPt;
    stdsptr<BonePt> mRootPt;

    qsptr<BoneTransformAnimator> mTransformAnimator;
    QList<qsptr<Bone>> mChildBones;
    qptr<Bone> mParentBone;
    qptr<BonesBox> mParentBonesBox;
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
    qptr<Bone> mParentBone;
    qptr<Bone> mTipBone;
    QList<qptr<Bone>> mRootBones;
};

class BonesBox : public BoundingBox {
    friend class SelfRef;
public:
    void drawPixmapSk(SkCanvas * const canvas, GrContext * const grContext);
    void drawPixmapSk(SkCanvas * const canvas, SkPaint * const paint,
                      GrContext* const grContext);
    void drawCanvasControls(SkCanvas * const canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);

    BoundingBox *createNewDuplicate();

    Bone *getBoneAtAbsPos(const QPointF &absPos);

    Bone *getBoneAtRelPos(const QPointF &relPos);

    bool relPointInsidePath(const QPointF &relPos) const;

    bool SWT_isBonesBox() const;

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>>& list);

    void addBone(const qsptr<Bone> &bone);
    void removeBone(const qsptr<Bone> &bone);
    void drawHoveredSk(SkCanvas *canvas, const SkScalar &invScale);
protected:
    BonesBox();

    QList<qsptr<Bone>> mBones;
};

#endif // BONE_H
