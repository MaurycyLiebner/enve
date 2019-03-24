#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "durationrectangle.h"
#include "boxesgrouprenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(BoxesGroup* linkTarget) :
    BoxesGroup(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
    const QList<qsptr<BoundingBox>> &boxesList =
            linkTarget->getContainedBoxesList();
    for(const auto& child : boxesList) {
        qsptr<BoundingBox> newLink = child->createLinkForLinkGroup();
        addContainedBox(newLink);
    }
    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

InternalLinkGroupBox::~InternalLinkGroupBox() {
    setLinkTarget(nullptr);
}

void InternalLinkGroupBox::writeBoundingBox(QIODevice *target) {
    BoxesGroup::writeBoundingBox(target);
    mBoxTarget->writeProperty(target);
}

void InternalLinkGroupBox::readBoundingBox(QIODevice *target) {
    BoxesGroup::readBoundingBox(target);
    mBoxTarget->readProperty(target);
}

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) {
//    return mLinkTarget->relPointInsidePath(point);
//}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelFrameRange(
        const int &relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            range *= BoundingBox::prp_getIdenticalRelFrameRange(relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheLeft();
            }
        }
    }
    auto targetRange = getLinkTarget()->prp_getIdenticalRelFrameRange(relFrame);

    return range*targetRange;
}

bool InternalLinkGroupBox::SWT_isBoxesGroup() const { return false; }

QMatrix InternalLinkGroupBox::getRelativeTransformAtRelFrameF(
        const qreal &relFrame) {
    if(mParentGroup ? mParentGroup->SWT_isLinkBox() : false) {
        return getLinkTarget()->getRelativeTransformAtRelFrameF(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtRelFrameF(relFrame);
    }
}

void InternalLinkGroupBox::setupEffectsF(const qreal &relFrame,
                                         BoundingBoxRenderData *data) {
    if(mParentGroup->SWT_isLinkBox()) {
        getLinkTarget()->setupEffectsF(relFrame, data);
    } else {
        BoundingBox::setupEffectsF(relFrame, data);
    }
}

qreal InternalLinkGroupBox::getEffectsMarginAtRelFrameF(const qreal &relFrame) {
if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->getEffectsMarginAtRelFrameF(relFrame);
    }
    return BoxesGroup::getEffectsMarginAtRelFrameF(relFrame);
}

const SkBlendMode &InternalLinkGroupBox::getBlendMode() {
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->getBlendMode();
    }
    return BoundingBox::getBlendMode();
}

void InternalLinkGroupBox::setupBoundingBoxRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData *data) {
    if(getLinkTarget())
        getLinkTarget()->BoundingBox::
                setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    BoxesGroup::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
}

BoxesGroup *InternalLinkGroupBox::getFinalTarget() const {
    if(getLinkTarget()->SWT_isLinkBox()) {
        return GetAsPtr(getLinkTarget(), InternalLinkGroupBox)->getFinalTarget();
    }
    return getLinkTarget();
}

int InternalLinkGroupBox::prp_getRelFrameShift() const {
    if(getLinkTarget()->SWT_isLinkBox() ||
       (mParentGroup ? mParentGroup->SWT_isLinkBox() : false)) {
        return BoxesGroup::prp_getRelFrameShift() +
                getLinkTarget()->prp_getRelFrameShift();
    }
    return BoxesGroup::prp_getRelFrameShift();
}

bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getFinalTarget()) return false;
    return getFinalTarget()->relPointInsidePath(relPos);
}

void InternalLinkGroupBox::setTargetSlot(BoundingBox *target) {
    if(target->SWT_isBoxesGroup())
        setLinkTarget(GetAsPtr(target, BoxesGroup));
}

void InternalLinkGroupBox::setLinkTarget(BoxesGroup *linkTarget) {
    disconnect(mBoxTarget.data(), nullptr, this, nullptr);
    if(getLinkTarget()) {
        disconnect(getLinkTarget(), nullptr, this, nullptr);
        getLinkTarget()->removeLinkingBox(this);
    }
    if(!linkTarget) {
        setName("empty link");
        mBoxTarget->setTarget(nullptr);
    } else {
        setName(linkTarget->getName() + " link");
        mBoxTarget->setTarget(linkTarget);
        linkTarget->addLinkingBox(this);
        connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, &BoundingBox::prp_updateAfterChangedRelFrameRange);
    }
    scheduleUpdate(Animator::USER_CHANGE);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkGroupBox::setTargetSlot);
}

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    return getLinkTarget()->getRelCenterPosition();
}

BoxesGroup *InternalLinkGroupBox::getLinkTarget() const {
    return GetAsPtr(mBoxTarget->getTarget(), BoxesGroup);
}

qsptr<BoundingBox> InternalLinkGroupBox::createLink() {
    return getLinkTarget()->createLink();
}

qsptr<BoundingBox> InternalLinkGroupBox::createLinkForLinkGroup() {
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->createLinkForLinkGroup();
    } else {
        return SPtrCreate(InternalLinkGroupBox)(this);
    }
}

bool InternalLinkGroupBox::SWT_isLinkBox() const { return true; }

bool InternalLinkGroupBox::isRelFrameInVisibleDurationRect(const int &relFrame) const {
    if(!getLinkTarget()) return false;
    return BoxesGroup::isRelFrameInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
}

stdsptr<BoundingBoxRenderData> InternalLinkGroupBox::createRenderData() {
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

QRectF InternalLinkGroupBox::getRelBoundingRectAtRelFrame(const qreal &relFrame) {
    return getLinkTarget()->getRelBoundingRectAtRelFrame(relFrame);
}
