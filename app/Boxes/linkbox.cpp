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
    renderData->parentBox = this;
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

void InternalLinkBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    int fIdLT;
    int lIdLT;

    getLinkTarget()->prp_getFirstAndLastIdenticalRelFrame(&fIdLT,
                                                      &lIdLT,
                                                      relFrame);
    int fId;
    int lId;
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            BoundingBox::prp_getFirstAndLastIdenticalRelFrame(&fId,
                                                            &lId,
                                                            relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                fId = mDurationRectangle->getMaxFrameAsRelFrame();
                lId = INT_MAX;
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                fId = INT_MIN;
                lId = mDurationRectangle->getMinFrameAsRelFrame();
            }
        }
    } else {
        fId = INT_MIN;
        lId = INT_MAX;
    }
    fId = qMax(fId, fIdLT);
    lId = qMin(lId, lIdLT);
    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}

bool InternalLinkBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                      const int &relFrame2) {
    bool differences =
            ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1,
                                                             relFrame2) ||
            getLinkTarget()->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
    if(differences || mDurationRectangle == nullptr) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
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

void InternalLinkGroupBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    int fIdLT;
    int lIdLT;
    getLinkTarget()->prp_getFirstAndLastIdenticalRelFrame(&fIdLT,
                                                      &lIdLT,
                                                      relFrame);
    int fId;
    int lId;
    if(mVisible) {
        if(isRelFrameInVisibleDurationRect(relFrame)) {
            Animator::prp_getFirstAndLastIdenticalRelFrame(&fId,
                                                            &lId,
                                                            relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxFrameAsRelFrame()) {
                fId = mDurationRectangle->getMaxFrameAsRelFrame();
                lId = INT_MAX;
            } else if(relFrame < mDurationRectangle->getMinFrameAsRelFrame()) {
                fId = INT_MIN;
                lId = mDurationRectangle->getMinFrameAsRelFrame();
            }
        }
    } else {
        fId = INT_MIN;
        lId = INT_MAX;
    }
    fId = qMax(fId, fIdLT);
    lId = qMin(lId, lIdLT);
    if(lId > fId) {
        *firstIdentical = fId;
        *lastIdentical = lId;
    } else {
        *firstIdentical = relFrame;
        *lastIdentical = relFrame;
    }
}

bool InternalLinkGroupBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                           const int &relFrame2) {
    bool differences =
            ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1,
                                                             relFrame2) ||
            getLinkTarget()->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
    if(differences || mDurationRectangle == nullptr) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
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
    renderData->parentBox = this;
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
    if(renderedToImage) return;
    renderedToImage = true;
    QMatrix scale;
    scale.scale(resolution, resolution);
    QMatrix transformRes = transform*scale;
    //transformRes.scale(resolution, resolution);
    QRectF globalBoundingRect =
            transformRes.mapRect(relBoundingRect).
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);
    if(maxBoundsEnabled) {
        globalBoundingRect = globalBoundingRect.intersected(
                    scale.mapRect(maxBoundsRect));
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
        rasterCanvas->drawRect(QRectFToSkRect(relBoundingRect), fillP);
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
        path.addRect(QRectFToSkRect(relBoundingRect));
        path.toggleInverseFillType();
        rasterCanvas->drawPath(path, paintT);
        rasterCanvas->restore();
    }
    rasterCanvas->flush();
    delete rasterCanvas;

    drawPos = SkPoint::Make(qRound(globalBoundingRect.left()),
                            qRound(globalBoundingRect.top()));

    if(!pixmapEffects.isEmpty()) {
        foreach(const stdsptr<PixmapEffectRenderData>& effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, resolution);
        }
        clearPixmapEffects();
    }

    bitmap.setImmutable();
    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}
