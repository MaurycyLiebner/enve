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

InternalLinkBox::InternalLinkBox(BoundingBox* linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), SIGNAL(targetSet(BoundingBox*)),
            this, SLOT(setTargetSlot(BoundingBox*)));
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point) {
    return getLinkTarget()->relPointInsidePath(point);
}

bool InternalLinkBox::isRelFrameInVisibleDurationRect(const int &relFrame) {
    if(getLinkTarget() == nullptr) return false;
    return BoundingBox::isRelFrameInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameInVisibleDurationRect(relFrame);
}

bool InternalLinkBox::isRelFrameFInVisibleDurationRect(const qreal &relFrame) {
    if(getLinkTarget() == nullptr) return false;
    return BoundingBox::isRelFrameFInVisibleDurationRect(relFrame) &&
            getLinkTarget()->isRelFrameFInVisibleDurationRect(relFrame);
}

FrameRange InternalLinkBox::prp_getIdenticalRelFrameRange(const int &relFrame) {
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
    connect(mBoxTarget.data(), SIGNAL(targetSet(BoundingBox*)),
            this, SLOT(setTargetSlot(BoundingBox*)));
}

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &point) {
//    return mLinkTarget->relPointInsidePath(point);
//}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelFrameRange(const int &relFrame) {
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
    canvasData->bgColor = QColorToSkColor(canvasTarget->getBgColorAnimator()->
            getColorAtRelFrameF(relFrame));
    //qreal res = getParentCanvas()->getResolutionFraction();
    canvasData->canvasHeight = canvasTarget->getCanvasHeight();//*res;
    canvasData->canvasWidth = canvasTarget->getCanvasWidth();//*res;
    if(mParentGroup->SWT_isLinkBox()) {
        canvasData->clipToCanvas =
                GetAsPtr(getLinkTarget(), InternalLinkCanvas)->clipToCanvas();
    } else {
        canvasData->clipToCanvas = mClipToCanvas->getValue();
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

bool InternalLinkCanvas::relPointInsidePath(const QPointF &relPos) {
    if(mClipToCanvas->getValue()) return mRelBoundingRect.contains(relPos);
    return InternalLinkGroupBox::relPointInsidePath(relPos);
}

void LinkCanvasRenderData::renderToImage() {
    if(fRenderedToImage) return;
    fRenderedToImage = true;
    QMatrix scale;
    scale.scale(fResolution, fResolution);
    QMatrix transformRes = fTransform*scale;
    //transformRes.scale(resolution, resolution);
    QRectF globalBoundingRect =
            transformRes.mapRect(fRelBoundingRect).
            adjusted(-fEffectsMargin, -fEffectsMargin,
                     fEffectsMargin, fEffectsMargin);
    if(fMaxBoundsEnabled) {
        globalBoundingRect = globalBoundingRect.intersected(
                    scale.mapRect(fMaxBoundsRect));
    }
    QSizeF sizeF = globalBoundingRect.size();
    QPointF transF = globalBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(globalBoundingRect.left()/**resolution*/),
                    qRound(globalBoundingRect.top()/**resolution*/));

    SkImageInfo info = SkImageInfo::Make(qCeil(sizeF.width()),
                                         qCeil(sizeF.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);//rasterSurface->getCanvas();
    //rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas->translate(static_cast<SkScalar>(-globalBoundingRect.left()),
                            static_cast<SkScalar>(-globalBoundingRect.top()));

    globalBoundingRect.translate(-transF);

    rasterCanvas->translate(static_cast<SkScalar>(transF.x()),
                            static_cast<SkScalar>(transF.y()));

    if(clipToCanvas) {
        rasterCanvas->save();
        rasterCanvas->concat(QMatrixToSkMatrix(transformRes));
        SkPaint fillP;
        fillP.setAntiAlias(true);
        fillP.setColor(bgColor);
        rasterCanvas->drawRect(QRectFToSkRect(fRelBoundingRect), fillP);
        rasterCanvas->restore();
    }

    drawSk(rasterCanvas);
    if(clipToCanvas) {
        rasterCanvas->save();
        rasterCanvas->concat(QMatrixToSkMatrix(transformRes));
        SkPaint paintT;
        paintT.setBlendMode(SkBlendMode::kDstIn);
        paintT.setColor(SK_ColorTRANSPARENT);
        paintT.setAntiAlias(true);
        SkPath path;
        path.addRect(QRectFToSkRect(fRelBoundingRect));
        path.toggleInverseFillType();
        rasterCanvas->drawPath(path, paintT);
        rasterCanvas->restore();
    }
    rasterCanvas->flush();
    delete rasterCanvas;

    fDrawPos = SkPoint::Make(qRound(globalBoundingRect.left()),
                            qRound(globalBoundingRect.top()));

    if(!fPixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, fPixmapEffects) {
            effect->applyEffectsSk(bitmap, fResolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}
