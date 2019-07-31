#include "linkbox.h"
#include <QFileDialog>
#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "skia/skiahelpers.h"

ExternalLinkBox::ExternalLinkBox() : ContainerBox(TYPE_LAYER) {
    mType = TYPE_EXTERNAL_LINK;
    prp_setName("Link Empty");
}

void ExternalLinkBox::reload() {


    planScheduleUpdate(Animator::USER_CHANGE);
}

void ExternalLinkBox::changeSrc(QWidget* dialogParent) {
    QString src = QFileDialog::getOpenFileName(dialogParent,
                                               "Link File",
                                               "",
                                               "enve Files (*.ev)");
    if(!src.isEmpty()) setSrc(src);
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    prp_setName("Link " + src);
    reload();
}

InternalLinkBox::InternalLinkBox(BoundingBox * const linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkBox::setTargetSlot);
}

QPointF InternalLinkBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() const {
    return mBoxTarget->getTarget();
}

qsptr<BoundingBox> InternalLinkBox::createLink() {
    if(!getLinkTarget()) BoundingBox::createLink();
    return getLinkTarget()->createLink();
}

qsptr<BoundingBox> InternalLinkBox::createLinkForLinkGroup() {
    return SPtrCreate(InternalLinkBox)(this);
}

stdsptr<BoxRenderData> InternalLinkBox::createRenderData() {
    const auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupRenderData(
        const qreal relFrame, BoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->setupRenderData(relFrame, data);

    BoundingBox::setupRenderData(relFrame, data);
}

QRectF InternalLinkBox::getRelBoundingRect(const qreal relFrame) {
    if(!getLinkTarget()) return QRectF();
    return getLinkTarget()->getRelBoundingRect(relFrame);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

bool InternalLinkBox::isFrameFInDurationRect(const qreal relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameFInDurationRect(relFrame) &&
            getLinkTarget()->isFrameFInDurationRect(relFrame);
}

FrameRange InternalLinkBox::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
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
