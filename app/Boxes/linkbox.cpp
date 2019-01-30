#include "linkbox.h"
#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "canvas.h"
#include "durationrectangle.h"
#include "PixmapEffects/rastereffects.h"
#include "Animators/transformanimator.h"
#include "PixmapEffects/pixmapeffect.h"

ExternalLinkBox::ExternalLinkBox() :
    BoxesGroup() {
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
    if(!src.isEmpty()) {
        setSrc(src);
    }
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

QRectF InternalLinkBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
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
    FrameRange range{INT_MIN, INT_MAX};
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
    foreach(const qsptr<BoundingBox> &child, boxesList) {
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
    FrameRange range{INT_MIN, INT_MAX};
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

QRectF InternalLinkGroupBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    return getLinkTarget()->getRelBoundingRectAtRelFrame(relFrame);
}

InternalLinkCanvas::InternalLinkCanvas(BoxesGroup* linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    ca_prependChildAnimator(mTransformAnimator.data(),
                            mClipToCanvas);
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
    canvasData->fBgColor = QColorToSkColor(canvasTarget->getBgColorAnimator()->
            getColorAtRelFrameF(relFrame));
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

void LinkCanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    QMatrix scale;
    scale.scale(fResolution, fResolution);
    fScaledTransform = fTransform*scale;
    //transformRes.scale(resolution, resolution);
    fGlobalBoundingRect =
            fScaledTransform.mapRect(fRelBoundingRect).
            adjusted(-fEffectsMargin, -fEffectsMargin,
                     fEffectsMargin, fEffectsMargin);
    if(fMaxBoundsEnabled) {
        fGlobalBoundingRect = fGlobalBoundingRect.intersected(
                    scale.mapRect(fMaxBoundsRect));
    }
    QSizeF sizeF = fGlobalBoundingRect.size();
    QPointF transF = fGlobalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(fGlobalBoundingRect.left()/**resolution*/),
                    qRound(fGlobalBoundingRect.top()/**resolution*/));

    SkImageInfo info = SkImageInfo::Make(qCeil(sizeF.width()),
                                         qCeil(sizeF.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas rasterCanvas(bitmap);//rasterSurface->getCanvas();
    //rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas.translate(static_cast<SkScalar>(-fGlobalBoundingRect.left()),
                           static_cast<SkScalar>(-fGlobalBoundingRect.top()));

    fGlobalBoundingRect.translate(-transF);

    rasterCanvas.translate(static_cast<SkScalar>(transF.x()),
                           static_cast<SkScalar>(transF.y()));

    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(QMatrixToSkMatrix(fScaledTransform));
        SkPaint fillP;
        fillP.setAntiAlias(true);
        fillP.setColor(fBgColor);
        rasterCanvas.drawRect(QRectFToSkRect(fRelBoundingRect), fillP);
        rasterCanvas.restore();
    }

    drawSk(&rasterCanvas);
    if(fClipToCanvas) {
        rasterCanvas.save();
        rasterCanvas.concat(QMatrixToSkMatrix(fScaledTransform));
        SkPaint paintT;
        paintT.setBlendMode(SkBlendMode::kDstIn);
        paintT.setColor(SK_ColorTRANSPARENT);
        paintT.setAntiAlias(true);
        SkPath path;
        path.addRect(QRectFToSkRect(fRelBoundingRect));
        path.toggleInverseFillType();
        rasterCanvas.drawPath(path, paintT);
        rasterCanvas.restore();
    }
    rasterCanvas.flush();

    fDrawPos = SkPoint::Make(qRound(fGlobalBoundingRect.left()),
                             qRound(fGlobalBoundingRect.top()));

    if(!fPixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, fPixmapEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    fRenderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}
