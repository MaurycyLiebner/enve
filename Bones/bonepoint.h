#ifndef BONEPOINT_H
#define BONEPOINT_H
#include "movablepoint.h"

class WeightMap {
public:
    enum PaintMode : short {
        PAINT_ADD,
        PAINT_SUBSTRACT,
        PAINT_REPLACE
    };

    void paintAt(const qreal &x,
                 const qreal &y,
                 const qreal &radius,
                 const qreal &weight,
                 const qreal &hardness,
                 const qreal &opacity,
                 const PaintMode &paintMode);

    void setSize(const int &width, const int &height);
    static void setWeightImageColorTable(QImage *img);
private:
    QImage mWeightMapImg;
};

class PathWeightMap {
public:

    void setNumberOfControlPoints(const int &nPoints);
    void setPointWeight(const int &pointId, const qreal &weight);
    void applyToPath(QPainterPath *path);
    void startPath(const QPointF &p);
    void addPoint(const QPointF &c1,
                  const QPointF &c2,
                  const QPointF &p);
    QPainterPath finishPath();
private:
    QPointF mLastPoint;
    QPainterPath mPath;
    qreal *mControlPointsWeight = NULL;
    int mNumberOfControlPoints;
};
#include "Properties/boolproperty.h"
#include "Animators/transformanimator.h"
class Bone : public ComplexAnimator {
public:
    Bone(BonePoint *rootPoint,
         BonePoint *tipPoint) {
        mRootPoint = rootPoint->ref<BonePoint>();
        mTipPoint = tipPoint->ref<BonePoint>();
        prp_setName("bone");

        mFixedLength->prp_setName("fixed length");
        ca_addChildAnimator(mFixedLength.data());
        ca_addChildAnimator(mTransformAnimator.data());
        mRootPoint->prp_setName("root");
        mTipPoint->prp_setName("tip");
        ca_addChildAnimator(mRootPoint.data());
        ca_addChildAnimator(mTipPoint.data());
    }

    QPointF getCurrentRootAbsPos() {
        return mRootPoint->getAbsolutePos();
    }

    QPointF getCurrentTipAbsPos() {
        return mTipPoint->getAbsolutePos();
    }

    qreal getCurrentRotation() {
        degreesBetweenVectors(
                    mTipPoint->getAbsolutePos() - mRootPoint->getAbsolutePos(),
                    QPointF(1., 0.));
    }

    void moveTipPointToAbsPos(const QPointF &absPos) {
        if(mFixedLength->getValue()) {
            QPointF tipVect = absPos - mRootPoint->getCurrentPointValue();
            qreal currLen = pointToLen(mTipPoint->getCurrentPointValue() -
                                       mRootPoint->getCurrentPointValue());
            tipVect = scalePointToNewLen(tipVect, currLen);
            mTipPoint->moveToAbs(mRootPoint->getCurrentPointValue() + tipVect);
        } else {
            mTranslationAnimator->setCurrentPointValue(
                        mTranslationAnimator->getCurrentPointValue()
                        absPos - mTipPoint->getAbsolutePos());
            mTipPoint->moveToAbs(absPos);
        }
    }


private:
    QSharedPointer<Bone> mParentBone;
    QMatrix mRelTransform;
    QMatrix mCombinedTransform;

    QSharedPointer<BoolProperty> mFixedLength =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<BasicTransformAnimator> mTransformAnimator =
            (new BasicTransformAnimator())->ref<BasicTransformAnimator>();
    QSharedPointer<BonePoint> mRootPoint;
    QSharedPointer<BonePoint> mTipPoint;
};

class BonePoint : public MovablePoint {
public:
    BonePoint(BoundingBox *bonesBox);
    BonePoint(BoundingBox *bonesBox, BonePoint *parentBonePt);

    void addChildBonePoint(BonePoint *childBonePt);
    void setParentBonePointVar(BonePoint *parentBonePt);
    void setParentBonePoint(BonePoint *parentBonePt);
protected:
    QSharedPointer<QrealAnimator> mRotAnimator;
    QSharedPointer<QPointFAnimator> mScaleAnimator;
    bool mBoneBeetweenThisAndPrev = false;
    QList<QSharedPointer<BonePoint> > mChildBonePoints;
    QSharedPointer<BonePoint> mParentBonePoint;
};

#endif // BONEPOINT_H
