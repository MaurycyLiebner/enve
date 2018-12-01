#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/boolproperty.h"

class ExternalLinkBox : public BoxesGroup {
    Q_OBJECT
    friend class SelfRef;
public:
    void reload();

    void changeSrc();

    void setSrc(const QString &src);
private:
    ExternalLinkBox();

    QString mSrc;
};

class InternalLinkBox : public BoundingBox {
    Q_OBJECT
    friend class SelfRef;
public:
    ~InternalLinkBox() {
        setLinkTarget(nullptr);
    }

    void writeBoundingBox(QIODevice *target) {
        BoundingBox::writeBoundingBox(target);
        mBoxTarget->writeProperty(target);
    }

    void readBoundingBox(QIODevice *target) {
        BoundingBox::readBoundingBox(target);
        mBoxTarget->readProperty(target);
    }

    void setLinkTarget(BoundingBox *linkTarget) {
        disconnect(mBoxTarget.data(), nullptr, this, nullptr);
        if(getLinkTarget() != nullptr) {
            disconnect(getLinkTarget(), nullptr, this, nullptr);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(linkTarget == nullptr) {
            setName("empty link");

            mBoxTarget->setTarget(nullptr);
        } else {
            setName(linkTarget->getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate(Animator::USER_CHANGE);
        connect(mBoxTarget.data(), SIGNAL(targetSet(qsptr<BoundingBox>)),
                this, SLOT(setTargetSlot(qsptr<BoundingBox>)));
    }

    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoundingBox *getLinkTarget() const;

    qsptr<BoundingBox> createLink();

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data);
    const SkBlendMode &getBlendMode() {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getBlendMode();
        }
        return BoundingBox::getBlendMode();
    }

    qreal getEffectsMarginAtRelFrame(const int &relFrame) {
        qreal margin = 0.;
        margin += getLinkTarget()->getEffectsMarginAtRelFrame(relFrame);
        margin += BoundingBox::getEffectsMarginAtRelFrame(relFrame);
        return margin;
    }

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    bool SWT_isLinkBox() { return true; }

    QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        if(mParentGroup == nullptr ? false : mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }
    bool isRelFrameInVisibleDurationRect(const int &relFrame);
    bool isRelFrameFInVisibleDurationRect(const qreal &relFrame);
public slots:
    void setTargetSlot(BoundingBox *target) {
        setLinkTarget(target);
    }
protected:
    InternalLinkBox(BoundingBox *linkTarget);
    qsptr<BoxTargetProperty> mBoxTarget =
            SPtrCreate(BoxTargetProperty)("link target");
};

class InternalLinkGroupBox : public BoxesGroup {
    Q_OBJECT
    friend class SelfRef;
public:
    ~InternalLinkGroupBox() {
        setLinkTarget(nullptr);
    }

    void writeBoundingBox(QIODevice *target) {
        BoxesGroup::writeBoundingBox(target);
        mBoxTarget->writeProperty(target);
    }

    void readBoundingBox(QIODevice *target) {
        BoxesGroup::readBoundingBox(target);
        mBoxTarget->readProperty(target);
    }

    void setLinkTarget(BoxesGroup *linkTarget) {
        disconnect(mBoxTarget.data(), nullptr, this, nullptr);
        if(getLinkTarget() != nullptr) {
            disconnect(getLinkTarget(), nullptr, this, nullptr);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(linkTarget == nullptr) {
            setName("empty link");

            mBoxTarget->setTarget(nullptr);
        } else {
            setName(linkTarget->getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate(Animator::USER_CHANGE);
        connect(mBoxTarget.data(), SIGNAL(targetSet(qsptr<BoundingBox>)),
                this, SLOT(setTargetSlot(qsptr<BoundingBox>)));
    }

    //bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoxesGroup *getLinkTarget() const;

    qsptr<BoundingBox> createLink() {
        return getLinkTarget()->createLink();
    }

    qsptr<BoundingBox> createLinkForLinkGroup();

    bool SWT_isLinkBox() { return true; }

    qsptr<BoundingBox> createNewDuplicate() {
        return SPtrCreate(InternalLinkGroupBox)(getLinkTarget());
    }

    bool isRelFrameInVisibleDurationRect(const int &relFrame) {
        if(getLinkTarget() == nullptr) return false;
        return BoxesGroup::isRelFrameInVisibleDurationRect(relFrame) &&
                getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
    }

    stdsptr<BoundingBoxRenderData> createRenderData();
    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);

    bool SWT_isBoxesGroup() { return false; }

    QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        if(getLinkTarget()->SWT_isLinkBox()) {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame)*
                    getLinkTarget()->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }


    void setupEffectsF(const qreal &relFrame,
                       BoundingBoxRenderData* data) {
        if(mParentGroup->SWT_isLinkBox()) {
            getLinkTarget()->setupEffectsF(relFrame, data);
        } else {
            BoundingBox::setupEffectsF(relFrame, data);
        }
    }

    qreal getEffectsMarginAtRelFrame(const int &relFrame) {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getEffectsMarginAtRelFrame(relFrame);
        }
        return BoxesGroup::getEffectsMarginAtRelFrame(relFrame);
    }

    const SkBlendMode &getBlendMode() {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getBlendMode();
        }
        return BoundingBox::getBlendMode();
    }

    void setupBoundingBoxRenderDataForRelFrameF(
                            const qreal &relFrame,
                            BoundingBoxRenderData* data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
        auto groupData = GetAsPtr(data, BoxesGroupRenderData);
        groupData->childrenRenderData.clear();
        qreal childrenEffectsMargin = 0.;
        qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        foreach(const qsptr<BoundingBox> &box, mContainedBoxes) {
            qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
            if(box->isRelFrameFVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData* boxRenderData =
                        box->getCurrentRenderData(qRound(boxRelFrame));
                if(boxRenderData == nullptr) {
                    continue;
                }
                boxRenderData->addDependent(data);
                groupData->childrenRenderData <<
                        GetAsSPtr(boxRenderData, BoundingBoxRenderData);
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrameF(boxRelFrame),
                             childrenEffectsMargin);
            }
        }
        data->effectsMargin += childrenEffectsMargin;
    }

    BoxesGroup *getFinalTarget() {
        if(getLinkTarget()->SWT_isLinkBox()) {
            return GetAsPtr(getLinkTarget(), InternalLinkGroupBox)->getFinalTarget();
        }
        return getLinkTarget();
    }

    int prp_getRelFrameShift() const {
        if(getLinkTarget()->SWT_isLinkBox() ||
           (mParentGroup == nullptr ? false : mParentGroup->SWT_isLinkBox())) {
            return BoxesGroup::prp_getRelFrameShift() +
                    getLinkTarget()->prp_getRelFrameShift();
        }
        return BoxesGroup::prp_getRelFrameShift();
    }

    bool relPointInsidePath(const QPointF &relPos) {
        if(mRelBoundingRect.contains(relPos)) {
            QPointF relPosT = getLinkTarget()->getRelativeTransformAtCurrentFrame().
                    inverted().map(relPos);
            return getFinalTarget()->relPointInsidePath(relPosT);
        }
        return false;
    }
public slots:
    void setTargetSlot(BoundingBox *target) {
        if(target->SWT_isBoxesGroup()) {
            setLinkTarget(GetAsPtr(target, BoxesGroup));
        }
    }
protected:
    InternalLinkGroupBox(BoxesGroup *linkTarget);

    qsptr<BoxTargetProperty> mBoxTarget =
            SPtrCreate(BoxTargetProperty)("link target");
};

struct LinkCanvasRenderData : public CanvasRenderData {
    friend class StdSelfRef;

    void updateRelBoundingRect() {
        if(clipToCanvas) {
            CanvasRenderData::updateRelBoundingRect();
        } else {
            BoxesGroupRenderData::updateRelBoundingRect();
        }
    }

    bool clipToCanvas = false;
protected:
    LinkCanvasRenderData(BoundingBox* parentBoxT) :
        CanvasRenderData(parentBoxT) {}

    void renderToImage();
};

class InternalLinkCanvas : public InternalLinkGroupBox {
    Q_OBJECT
    friend class SelfRef;
public:
    void addSchedulersToProcess();

    void writeBoundingBox(QIODevice *target);

    void readBoundingBox(QIODevice *target);

    void processSchedulers();

    void setupBoundingBoxRenderDataForRelFrameF(
                            const qreal &relFrame,
                            BoundingBoxRenderData* data);
    bool clipToCanvas();

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoundingBoxRenderData> createRenderData();

    bool relPointInsidePath(const QPointF &relPos);
protected:
    InternalLinkCanvas(BoxesGroup* linkTarget);
    qsptr<BoolProperty> mClipToCanvas =
            SPtrCreate(BoolProperty)("clip");
};

#endif // LINKBOX_H
