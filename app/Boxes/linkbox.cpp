#include "linkbox.h"
#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "canvas.h"
#include "durationrectangle.h"
#include "PixmapEffects/rastereffects.h"
#include "Animators/transformanimator.h"
#include "PixmapEffects/pixmapeffect.h"
#include "skia/skiahelpers.h"

ExternalLinkBox::ExternalLinkBox() : BoxesGroup() {
    mType = TYPE_EXTERNAL_LINK;
    setName("Link Empty");
}

void ExternalLinkBox::reload() {


    scheduleUpdate(Animator::USER_CHANGE);
}

void ExternalLinkBox::changeSrc(QWidget* dialogParent) {
    QString src = QFileDialog::getOpenFileName(dialogParent,
                                               "Link File",
                                               "",
                                               "AniVect Files (*.av)");
    if(!src.isEmpty()) setSrc(src);
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    setName("Link " + src);
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

stdsptr<BoundingBoxRenderData> InternalLinkBox::createRenderData() {
    const auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupRenderData(
        const qreal &relFrame, BoundingBoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) {
        linkTarget->setupRenderData(relFrame, data);
    }

    BoundingBox::setupRenderData(relFrame, data);
    if(linkTarget) {
        const qreal targetMargin =
                linkTarget->getEffectsMarginAtRelFrameF(relFrame);
        data->fEffectsMargin += targetMargin*data->fResolution;
    }
}

QRectF InternalLinkBox::getRelBoundingRect(const qreal &relFrame) {
    if(!getLinkTarget()) return QRectF();
    return getLinkTarget()->getRelBoundingRect(relFrame);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkBox::isRelFrameInVisibleDurationRect(const int &relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isRelFrameInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
}

bool InternalLinkBox::isRelFrameFInVisibleDurationRect(const qreal &relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isRelFrameFInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameFInVisibleDurationRect(relFrame);
}

FrameRange InternalLinkBox::prp_getIdenticalRelFrameRange(const int &relFrame) const {
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
