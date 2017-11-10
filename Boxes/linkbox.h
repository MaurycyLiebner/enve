#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"
#include "Properties/boxtargetproperty.h"

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
    ~InternalLinkBox() {
        setLinkTarget(NULL);
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
        disconnect(mBoxTarget.data(), 0, this, 0);
        if(getLinkTarget() != NULL) {
            disconnect(getLinkTarget(), 0, this, 0);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(linkTarget == NULL) {
            setName("empty link");

            mBoxTarget->setTarget(NULL);
        } else {
            setName(linkTarget->getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate();
        connect(mBoxTarget.data(), SIGNAL(targetSet(BoundingBox*)),
                this, SLOT(setTargetSlot(BoundingBox*)));
    }

    bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoundingBox *getLinkTarget() const;

    BoundingBox *createLink();

    BoundingBox *createLinkForLinkGroup() {
        return new InternalLinkBox(this);
    }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkBox(getLinkTarget());
    }

    BoundingBoxRenderData *createRenderData();
    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

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
        if(mParentGroup == NULL ? false : mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }
    bool isRelFrameInVisibleDurationRect(const int &relFrame);
public slots:
    void setTargetSlot(BoundingBox *target) {
        setLinkTarget(target);
    }
protected:
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
};

class InternalLinkGroupBox : public BoxesGroup {
    Q_OBJECT
public:
    InternalLinkGroupBox(BoxesGroup *linkTarget);
    ~InternalLinkGroupBox() {
        setLinkTarget(NULL);
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
        disconnect(mBoxTarget.data(), 0, this, 0);
        if(getLinkTarget() != NULL) {
            disconnect(getLinkTarget(), 0, this, 0);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(linkTarget == NULL) {
            setName("empty link");

            mBoxTarget->setTarget(NULL);
        } else {
            setName(linkTarget->getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, SIGNAL(prp_absFrameRangeChanged(int,int)),
                    this, SLOT(prp_updateAfterChangedRelFrameRange(int,int)));
        }
        scheduleUpdate();
        connect(mBoxTarget.data(), SIGNAL(targetSet(BoundingBox*)),
                this, SLOT(setTargetSlot(BoundingBox*)));
    }

    //bool relPointInsidePath(const QPointF &point);
    QPointF getRelCenterPosition();

    BoxesGroup *getLinkTarget() const;

    BoundingBox *createLink() {
        return getLinkTarget()->createLink();
    }

    BoundingBox *createLinkForLinkGroup() {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->createLinkForLinkGroup();
        } else {
            return new InternalLinkGroupBox(this);
        }
    }

    bool SWT_isLinkBox() { return true; }

    BoundingBox *createNewDuplicate() {
        return new InternalLinkGroupBox(getLinkTarget());
    }

    bool isRelFrameInVisibleDurationRect(const int &relFrame) {
        if(getLinkTarget() == NULL) return false;
        return BoxesGroup::isRelFrameInVisibleDurationRect(relFrame) &&
                getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
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
        if(getLinkTarget()->SWT_isLinkBox()) {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame)*
                    getLinkTarget()->getRelativeTransformAtRelFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrame(relFrame);
        }
    }

    void setupEffects(const int &relFrame,
                      BoundingBoxRenderData *data) {
        if(mParentGroup->SWT_isLinkBox()) {
            getLinkTarget()->setupEffects(relFrame, data);
        } else {
            BoundingBox::setupEffects(relFrame, data);
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

    void setupBoundingBoxRenderDataForRelFrame(
                            const int &relFrame,
                            BoundingBoxRenderData *data) {
        BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                           data);
        BoxesGroupRenderData *groupData = ((BoxesGroupRenderData*)data);
        groupData->childrenRenderData.clear();
        qreal childrenEffectsMargin = 0.;
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        foreach(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
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
        if(getLinkTarget()->SWT_isLinkBox()) {
            return ((InternalLinkGroupBox*)getLinkTarget())->getFinalTarget();
        }
        return getLinkTarget();
    }

    int prp_getRelFrameShift() const {
        if(getLinkTarget()->SWT_isLinkBox() ||
           (mParentGroup == NULL ? false : mParentGroup->SWT_isLinkBox())) {
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
            setLinkTarget((BoxesGroup*)target);
        }
    }
protected:
    QSharedPointer<BoxTargetProperty> mBoxTarget =
            (new BoxTargetProperty())->ref<BoxTargetProperty>();
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
        getLinkTarget()->addSchedulersToProcess();
        BoxesGroup::addSchedulersToProcess();
    }

    void writeBoundingBox(QIODevice *target) {
        InternalLinkGroupBox::writeBoundingBox(target);
        mClipToCanvas->writeProperty(target);
    }

    void readBoundingBox(QIODevice *target) {
        InternalLinkGroupBox::readBoundingBox(target);
        mClipToCanvas->readProperty(target);
    }

    void processSchedulers() {
        getLinkTarget()->processSchedulers();
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
        qreal res = getParentCanvas()->getResolutionFraction();
        canvasData->canvasHeight = canvasTarget->getCanvasHeight()*res;
        canvasData->canvasWidth = canvasTarget->getCanvasWidth()*res;
        if(mParentGroup->SWT_isLinkBox()) {
            canvasData->clipToCanvas =
                    ((InternalLinkCanvas*)getLinkTarget())->clipToCanvas();
        } else {
            canvasData->clipToCanvas = mClipToCanvas->getValue();
        }
    }

    bool clipToCanvas() {
        return mClipToCanvas->getValue();
    }

    BoundingBox *createLinkForLinkGroup() {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->createLinkForLinkGroup();
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
