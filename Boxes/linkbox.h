#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"

class ExternalLinkBox : public BoxesGroup
{
    Q_OBJECT
public:
    ExternalLinkBox();
    void reload();

    void changeSrc();

    void setSrc(const QString &src);

    BoundingBox *createNewDuplicate() {
        return new ExternalLinkBox();
    }

    void makeDuplicate(Property *targetBox) {
        BoundingBox::makeDuplicate(targetBox);
        ((ExternalLinkBox*)targetBox)->setSrc(mSrc);
    }

private:
    QString mSrc;
};

class InternalLinkBox : public BoundingBox {
    Q_OBJECT
public:
    InternalLinkBox(BoundingBox *linkTarget);

    void setLinkTarget(BoundingBox *linkTarget) {
        if(mLinkTarget != NULL) {
            disconnect(mLinkTarget.data(), 0, this, 0);
        }
        if(linkTarget == NULL) {
            setName("empty link");

            mLinkTarget.reset();
        } else {
            setName(linkTarget->getName() + " link");
            mLinkTarget = linkTarget->ref<BoundingBox>();
//            connect(linkTarget, SIGNAL(scheduledUpdate()),
//                    this, SLOT(scheduleUpdate()));
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate();
    }

    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoundingBox *getLinkTarget();

    BoundingBox *createLink();

    BoundingBox *createLinkForLinkGroup() {
        return new InternalLinkBox(this);
    }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkBox(mLinkTarget.data());
    }

    BoundingBoxRenderData *createRenderData();
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

    qreal getEffectsMarginAtRelFrame(const int &relFrame) {
        qreal margin = 0.;
        margin += mLinkTarget->getEffectsMarginAtRelFrame(relFrame);
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
        if(mParent == NULL ? false : mParent->SWT_isLinkBox()) {
            return mLinkTarget->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }
    bool isRelFrameInVisibleDurationRect(const int &relFrame);
protected:
    QSharedPointer<BoundingBox> mLinkTarget;
};

class InternalLinkGroupBox : public BoxesGroup {
    Q_OBJECT
public:
    InternalLinkGroupBox(BoxesGroup *linkTarget);

    void setLinkTarget(BoxesGroup *linkTarget) {
        if(mLinkTarget != NULL) {
            disconnect(mLinkTarget.data(), 0, this, 0);
        }
        if(linkTarget == NULL) {
            setName("empty link");

            mLinkTarget.reset();
        } else {
            setName(linkTarget->getName() + " link");
            mLinkTarget = linkTarget->ref<BoxesGroup>();
//            connect(linkTarget, SIGNAL(scheduledUpdate()),
//                    this, SLOT(scheduleUpdate()));
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate();
    }

    //bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoxesGroup *getLinkTarget();

    BoundingBox *createLink() {
        return mLinkTarget->createLink();
    }

    BoundingBox *createLinkForLinkGroup() {
        if(mParent->SWT_isLinkBox()) {
            return mLinkTarget->createLinkForLinkGroup();
        } else {
            return new InternalLinkGroupBox(this);
        }
    }

    bool SWT_isLinkBox() { return true; }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkGroupBox(mLinkTarget.data());
    }

    bool isRelFrameInVisibleDurationRect(const int &relFrame) {
        return BoxesGroup::isRelFrameInVisibleDurationRect(relFrame) &&
                mLinkTarget->isRelFrameInVisibleDurationRect(relFrame);
    }

    BoundingBoxRenderData *createRenderData();
    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);

    bool SWT_isBoxesGroup() { return false; }

    QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        if(mLinkTarget->SWT_isLinkBox()) {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame)*
                    mLinkTarget->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }

    void setupEffects(const int &relFrame,
                      BoundingBoxRenderData *data) {
        if(mParent->SWT_isLinkBox()) {
            mLinkTarget->setupEffects(relFrame, data);
        } else {
            BoundingBox::setupEffects(relFrame, data);
        }
    }

    qreal getEffectsMarginAtRelFrame(const int &relFrame) {
        if(mParent->SWT_isLinkBox()) {
            return mLinkTarget->getEffectsMarginAtRelFrame(relFrame);
        }
        return BoxesGroup::getEffectsMarginAtRelFrame(relFrame);
    }

    void setupBoundingBoxRenderDataForRelFrame(
                            const int &relFrame,
                            BoundingBoxRenderData *data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                           data);
        BoxesGroupRenderData *groupData = ((BoxesGroupRenderData*)data);
        groupData->childrenRenderData.clear();
        qreal childrenEffectsMargin = 0.;
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        foreach(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            int boxRelFrame = box->prp_absFrameToRelFrame(absFrame);
            if(box->isRelFrameVisibleAndInVisibleDurationRect(boxRelFrame)) {
                BoundingBoxRenderData *boxRenderData =
                        box->getCurrentRenderData();
                if(boxRenderData == NULL) {
                    continue;
                }
                if(!boxRenderData->finished()) {
                    boxRenderData->addDependent(data);
                }
                groupData->childrenRenderData <<
                        boxRenderData->ref<BoundingBoxRenderData>();
                childrenEffectsMargin =
                        qMax(box->getEffectsMarginAtRelFrame(boxRelFrame),
                             childrenEffectsMargin);
            }
        }
        data->effectsMargin += childrenEffectsMargin;
    }

    BoxesGroup *getFinalTarget() {
        if(mLinkTarget->SWT_isLinkBox()) {
            return ((InternalLinkGroupBox*)mLinkTarget.data())->getFinalTarget();
        }
        return mLinkTarget.data();
    }

    int prp_getRelFrameShift() const {
        if(mLinkTarget->SWT_isLinkBox() ||
           (mParent == NULL ? false : mParent->SWT_isLinkBox())) {
            return BoxesGroup::prp_getRelFrameShift() +
                    mLinkTarget->prp_getRelFrameShift();
        }
        return BoxesGroup::prp_getRelFrameShift();
    }

    bool relPointInsidePath(const QPointF &relPos) {
        if(mRelBoundingRect.contains(relPos)) {
            QPointF relPosT = mLinkTarget->getRelativeTransformAtCurrentFrame().
                    inverted().map(relPos);
            return getFinalTarget()->relPointInsidePath(relPosT);
        }
        return false;
    }

protected:
    QSharedPointer<BoxesGroup> mLinkTarget;
};

struct LinkCanvasRenderData : public CanvasRenderData {
    LinkCanvasRenderData(BoundingBox *parentBoxT) :
        CanvasRenderData(parentBoxT) {

    }

    void updateRelBoundingRect() {
        if(clipToCanvas) {
            CanvasRenderData::updateRelBoundingRect();
        } else {
            BoxesGroupRenderData::updateRelBoundingRect();
        }
    }

    bool clipToCanvas = false;
protected:
    void renderToImage();
};

class InternalLinkCanvas : public InternalLinkGroupBox {
    Q_OBJECT
public:
    InternalLinkCanvas(BoxesGroup *linkTarget);
    void addSchedulersToProcess() {
        mLinkTarget->addSchedulersToProcess();
        BoxesGroup::addSchedulersToProcess();
    }

    void processSchedulers() {
        mLinkTarget->processSchedulers();
        BoxesGroup::processSchedulers();
    }

    void setupBoundingBoxRenderDataForRelFrame(
                            const int &relFrame,
                            BoundingBoxRenderData *data) {
        InternalLinkGroupBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                                    data);

        BoxesGroup *finalTarget = getFinalTarget();
        LinkCanvasRenderData *canvasData = (LinkCanvasRenderData*)data;
        Canvas *canvasTarget = (Canvas*)finalTarget;
        canvasData->bgColor = canvasTarget->getBgColorAnimator()->
                                getColorAtRelFrame(relFrame).getSkColor();
        canvasData->canvasHeight = canvasTarget->getCanvasHeight()*
                                    canvasTarget->getResolutionFraction();
        canvasData->canvasWidth = canvasTarget->getCanvasWidth()*
                                    canvasTarget->getResolutionFraction();
        if(mParent->SWT_isLinkBox()) {
            canvasData->clipToCanvas =
                    ((InternalLinkCanvas*)mLinkTarget.data())->clipToCanvas();
        } else {
            canvasData->clipToCanvas = mClipToCanvas->getValue();
        }
    }

    bool clipToCanvas() {
        return mClipToCanvas->getValue();
    }

    BoundingBox *createLinkForLinkGroup() {
        if(mParent->SWT_isLinkBox()) {
            return mLinkTarget->createLinkForLinkGroup();
        } else {
            return new InternalLinkCanvas(this);
        }
    }

    BoundingBoxRenderData *createRenderData() {
        return new LinkCanvasRenderData(this);
    }

    bool relPointInsidePath(const QPointF &relPos) {
        if(mClipToCanvas->getValue()) return mRelBoundingRect.contains(relPos);
        return InternalLinkGroupBox::relPointInsidePath(relPos);
    }
protected:
    QSharedPointer<BoolProperty> mClipToCanvas =
                (new BoolProperty())->ref<BoolProperty>();
};

#endif // LINKBOX_H
