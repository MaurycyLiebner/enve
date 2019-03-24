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

QPointF InternalLinkBox::getRelCenterPosition() {
    return getLinkTarget()->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() const {
    return mBoxTarget->getTarget();
}

qsptr<BoundingBox> InternalLinkBox::createLink() {
    return getLinkTarget()->createLink();
}

qsptr<BoundingBox> InternalLinkBox::createLinkForLinkGroup() {
    return SPtrCreate(InternalLinkBox)(this);
}

stdsptr<BoundingBoxRenderData> InternalLinkBox::createRenderData() {
    stdsptr<BoundingBoxRenderData> renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupBoundingBoxRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData *data) {
    getLinkTarget()->setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
}

QRectF InternalLinkBox::getRelBoundingRectAtRelFrame(const qreal &relFrame) {
    return getLinkTarget()->getRelBoundingRectAtRelFrame(relFrame);
}

InternalLinkBox::InternalLinkBox(BoundingBox * const linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkBox::setTargetSlot);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &relPos) const {
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkBox::isRelFrameInVisibleDurationRect(const int &relFrame) const {
    if(getLinkTarget() == nullptr) return false;
    return BoundingBox::isRelFrameInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
}

bool InternalLinkBox::isRelFrameFInVisibleDurationRect(const qreal &relFrame) const {
    if(getLinkTarget() == nullptr) return false;
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

InternalLinkGroupBox::InternalLinkGroupBox(BoxesGroup* linkTarget) :
    BoxesGroup() {
    mType = TYPE_INTERNAL_LINK;
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

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) {
//    return mLinkTarget->relPointInsidePath(point);
//}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelFrameRange(const int &relFrame) const {
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

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    return getLinkTarget()->getRelCenterPosition();
}

BoxesGroup *InternalLinkGroupBox::getLinkTarget() const {
    return GetAsPtr(mBoxTarget->getTarget(), BoxesGroup);
}

qsptr<BoundingBox> InternalLinkGroupBox::createLinkForLinkGroup() {
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->createLinkForLinkGroup();
    } else {
        return SPtrCreate(InternalLinkGroupBox)(this);
    }
}

stdsptr<BoundingBoxRenderData> InternalLinkGroupBox::createRenderData() {
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

QRectF InternalLinkGroupBox::getRelBoundingRectAtRelFrame(const qreal &relFrame) {
    return getLinkTarget()->getRelBoundingRectAtRelFrame(relFrame);
}

#include "linkcanvasrenderdata.h"
InternalLinkCanvas::InternalLinkCanvas(BoxesGroup* linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    ca_prependChildAnimator(mTransformAnimator.data(), mClipToCanvas);
}

void InternalLinkCanvas::queScheduledTasks() {
    getLinkTarget()->queScheduledTasks();
    BoxesGroup::queScheduledTasks();
}

void InternalLinkCanvas::writeBoundingBox(QIODevice *target) {
    InternalLinkGroupBox::writeBoundingBox(target);
    mClipToCanvas->writeProperty(target);
}

void InternalLinkCanvas::readBoundingBox(QIODevice *target) {
    InternalLinkGroupBox::readBoundingBox(target);
    mClipToCanvas->readProperty(target);
}

void InternalLinkCanvas::scheduleWaitingTasks() {
    getLinkTarget()->scheduleWaitingTasks();
    BoxesGroup::scheduleWaitingTasks();
}

void InternalLinkCanvas::setupBoundingBoxRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData* data) {
    InternalLinkGroupBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);

    BoxesGroup* finalTarget = getFinalTarget();
    auto canvasData = GetAsSPtr(data, LinkCanvasRenderData);
    qsptr<Canvas> canvasTarget = GetAsSPtr(finalTarget, Canvas);
    canvasData->fBgColor = toSkColor(canvasTarget->getBgColorAnimator()->
            getColorAtRelFrame(relFrame));
    //qreal res = getParentCanvas()->getResolutionFraction();
    canvasData->canvasHeight = canvasTarget->getCanvasHeight();//*res;
    canvasData->canvasWidth = canvasTarget->getCanvasWidth();//*res;
    if(mParentGroup->SWT_isLinkBox()) {
        const auto ilc = GetAsPtr(getLinkTarget(), InternalLinkCanvas);
        canvasData->fClipToCanvas = ilc->clipToCanvas();
    } else {
        canvasData->fClipToCanvas = mClipToCanvas->getValue();
    }
}

bool InternalLinkCanvas::clipToCanvas() {
    return mClipToCanvas->getValue();
}

qsptr<BoundingBox> InternalLinkCanvas::createLinkForLinkGroup() {
    if(mParentGroup->SWT_isLinkBox()) {
        return getLinkTarget()->createLinkForLinkGroup();
    } else {
        return SPtrCreate(InternalLinkCanvas)(this);
    }
}

stdsptr<BoundingBoxRenderData> InternalLinkCanvas::createRenderData() {
    return SPtrCreate(LinkCanvasRenderData)(this);
}

bool InternalLinkCanvas::relPointInsidePath(const QPointF &relPos) const {
    if(mClipToCanvas->getValue()) return mRelBoundingRect.contains(relPos);
    return InternalLinkGroupBox::relPointInsidePath(relPos);
}
