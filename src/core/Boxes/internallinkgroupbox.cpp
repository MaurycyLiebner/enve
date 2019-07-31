#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "Timeline/durationrectangle.h"
#include "layerboxrenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(ContainerBox * const linkTarget) :
    ContainerBox(TYPE_INTERNAL_LINK_GROUP) {
    setLinkTarget(linkTarget);

    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

InternalLinkGroupBox::~InternalLinkGroupBox() {
    setLinkTarget(nullptr);
}

void InternalLinkGroupBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
}

void InternalLinkGroupBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
}

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) {
//    return mLinkTarget->relPointInsidePath(point);
//}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelRange(
        const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(!getLinkTarget()) return range;
    if(mVisible) {
        if(isFrameInDurationRect(relFrame)) {
            range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheLeft();
            }
        }
    }
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);

    return range*targetRange;
}

QMatrix InternalLinkGroupBox::getRelativeTransformAtRelFrameF(
        const qreal relFrame) {
    if(isParentLink() && getLinkTarget()) {
        return getLinkTarget()->getRelativeTransformAtRelFrameF(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtRelFrameF(relFrame);
    }
}

void InternalLinkGroupBox::setupGPUEffectsF(const qreal relFrame,
                                         BoxRenderData * const data) {
    if(isParentLink() && getLinkTarget()) {
        getLinkTarget()->setupGPUEffectsF(relFrame, data);
    } else {
        BoundingBox::setupGPUEffectsF(relFrame, data);
    }
}

QMarginsF InternalLinkGroupBox::getEffectsMargin(const qreal relFrame) {
    if(isParentLink() && getLinkTarget()) {
        return getLinkTarget()->getEffectsMargin(relFrame);
    }
    return ContainerBox::getEffectsMargin(relFrame);
}

const SkBlendMode &InternalLinkGroupBox::getBlendMode() {
    if(isParentLink() && getLinkTarget()) {
        return getLinkTarget()->getBlendMode();
    }
    return BoundingBox::getBlendMode();
}

void InternalLinkGroupBox::setupRenderData(const qreal relFrame,
                                           BoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->BoundingBox::setupRenderData(relFrame, data);

    ContainerBox::setupRenderData(relFrame, data);
}

ContainerBox *InternalLinkGroupBox::getFinalTarget() const {
    if(!getLinkTarget()) return nullptr;
    if(getLinkTarget()->SWT_isLinkBox()) {
        return GetAsPtr(getLinkTarget(), InternalLinkGroupBox)->getFinalTarget();
    }
    return getLinkTarget();
}

int InternalLinkGroupBox::prp_getRelFrameShift() const {
    if(!getLinkTarget()) return ContainerBox::prp_getRelFrameShift();
    if(getLinkTarget()->SWT_isLinkBox() || isParentLink()) {
        return ContainerBox::prp_getRelFrameShift() +
                getLinkTarget()->prp_getRelFrameShift();
    }
    return ContainerBox::prp_getRelFrameShift();
}

bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getFinalTarget()) return false;
    return getFinalTarget()->relPointInsidePath(relPos);
}

void InternalLinkGroupBox::setTargetSlot(BoundingBox * const target) {
    if(target->SWT_isLayerBox())
        setLinkTarget(GetAsPtr(target, ContainerBox));
}

void InternalLinkGroupBox::setLinkTarget(ContainerBox * const linkTarget) {
    disconnect(mBoxTarget.data(), nullptr, this, nullptr);
    if(getLinkTarget()) {
        disconnect(getLinkTarget(), nullptr, this, nullptr);
        getLinkTarget()->removeLinkingBox(this);
        removeAllContainedBoxes();
    }
    if(linkTarget) {
        prp_setName(linkTarget->prp_getName() + " link");
        mBoxTarget->setTarget(linkTarget);
        linkTarget->addLinkingBox(this);
        connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, &BoundingBox::prp_afterChangedRelRange);

        const auto &boxesList = linkTarget->getContainedBoxes();
        for(const auto& child : boxesList) {
            const auto newLink = child->createLinkForLinkGroup();
            addContainedBox(newLink);
        }
    } else {
        prp_setName("empty link");
        mBoxTarget->setTarget(nullptr);
    }
    planScheduleUpdate(Animator::USER_CHANGE);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

ContainerBox *InternalLinkGroupBox::getLinkTarget() const {
    return GetAsPtr(mBoxTarget->getTarget(), ContainerBox);
}

qsptr<BoundingBox> InternalLinkGroupBox::createLink() {
    if(!getLinkTarget()) return createLink();
    return getLinkTarget()->createLink();
}

qsptr<BoundingBox> InternalLinkGroupBox::createLinkForLinkGroup() {
    if(!getLinkTarget()) return createLink();
    if(isParentLink()) {
        return getLinkTarget()->createLinkForLinkGroup();
    } else {
        return SPtrCreate(InternalLinkGroupBox)(this);
    }
}

bool InternalLinkGroupBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return ContainerBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

stdsptr<BoxRenderData> InternalLinkGroupBox::createRenderData() {
    if(!getLinkTarget()) return nullptr;
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

QRectF InternalLinkGroupBox::getRelBoundingRect(const qreal relFrame) {
    if(!getLinkTarget()) return QRectF();
    return getLinkTarget()->getRelBoundingRect(relFrame);
}
