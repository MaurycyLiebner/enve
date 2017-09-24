#include "linkbox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "canvas.h"
#include "durationrectangle.h"

ExternalLinkBox::ExternalLinkBox() :
    BoxesGroup() {
    setType(TYPE_EXTERNAL_LINK);
    setName("Link Empty");
}

void ExternalLinkBox::reload() {


    scheduleUpdate();
}

void ExternalLinkBox::changeSrc() {
    QString src = QFileDialog::getOpenFileName(mMainWindow,
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
    return mLinkTarget->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() {
    return mLinkTarget.data();
}

BoundingBox *InternalLinkBox::createLink() {
    return mLinkTarget->createLink();
}

BoundingBoxRenderData *InternalLinkBox::createRenderData() {
    BoundingBoxRenderData *renderData = mLinkTarget->createRenderData();
    renderData->parentBox = weakRef<BoundingBox>();
    return renderData;
}

void InternalLinkBox::setupBoundingBoxRenderDataForRelFrame(
        const int &relFrame, BoundingBoxRenderData *data) {
    mLinkTarget->setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
}

QRectF InternalLinkBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    return mLinkTarget->getRelBoundingRectAtRelFrame(relFrame);
}

InternalLinkBox::InternalLinkBox(BoundingBox *linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &point) {
    return mLinkTarget->relPointInsidePath(point);
}

bool InternalLinkBox::isRelFrameInVisibleDurationRect(const int &relFrame) {
    return BoundingBox::isRelFrameInVisibleDurationRect(relFrame) &&
            mLinkTarget->isRelFrameInVisibleDurationRect(relFrame);
}

void InternalLinkBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    int fIdLT;
    int lIdLT;

    mLinkTarget->prp_getFirstAndLastIdenticalRelFrame(&fIdLT,
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

bool InternalLinkBox::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                      const int &relFrame2) {
    bool differences =
            ComplexAnimator::prp_differencesBetweenRelFrames(relFrame1,
                                                             relFrame2) ||
            mLinkTarget->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
    if(differences || mDurationRectangle == NULL) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
}


InternalLinkGroupBox::InternalLinkGroupBox(BoxesGroup *linkTarget) :
    BoxesGroup() {
    mType = TYPE_INTERNAL_LINK;
    setLinkTarget(linkTarget);
    const QList<QSharedPointer<BoundingBox> > &boxesList =
            linkTarget->getChildBoxesList();
    foreach(const QSharedPointer<BoundingBox> &child, boxesList) {
        QSharedPointer<BoundingBox> newLink =
                child->createLinkForLinkGroup()->ref<BoundingBox>();
        addChild(newLink.data());
    }
}

//bool InternalLinkGroupBox::relPointInsidePath(const QPointF &point) {
//    return mLinkTarget->relPointInsidePath(point);
//}

void InternalLinkGroupBox::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                        int *lastIdentical,
                                                        const int &relFrame) {
    int fIdLT;
    int lIdLT;
    mLinkTarget->prp_getFirstAndLastIdenticalRelFrame(&fIdLT,
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
            mLinkTarget->prp_differencesBetweenRelFrames(relFrame1,
                                                         relFrame2);
    if(differences || mDurationRectangle == NULL) return differences;
    return mDurationRectangle->hasAnimationFrameRange();
}


QPointF InternalLinkGroupBox::getRelCenterPosition() {
    return mLinkTarget->getRelCenterPosition();
}

BoxesGroup *InternalLinkGroupBox::getLinkTarget() {
    return mLinkTarget.data();
}

BoundingBoxRenderData *InternalLinkGroupBox::createRenderData() {
    BoundingBoxRenderData *renderData = mLinkTarget->createRenderData();
    renderData->parentBox = weakRef<BoundingBox>();
    return renderData;
}

QRectF InternalLinkGroupBox::getRelBoundingRectAtRelFrame(const int &relFrame) {
    return mLinkTarget->getRelBoundingRectAtRelFrame(relFrame);
}

InternalLinkCanvas::InternalLinkCanvas(BoxesGroup *linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    mClipToCanvas->prp_setName("clip to canvas");
    ca_addChildAnimator(mClipToCanvas.data());
}

#include "PixmapEffects/fmt_filters.h"
void LinkCanvasRenderData::renderToImage() {
    if(renderedToImage) return;
    renderedToImage = true;
    QMatrix scale;
    scale.scale(resolution, resolution);
    QMatrix transformRes = transform*scale;
    //transformRes.scale(resolution, resolution);
    QRectF allUglyBoundingRect =
            transformRes.mapRect(relBoundingRect).
            adjusted(-effectsMargin, -effectsMargin,
                     effectsMargin, effectsMargin);
    allUglyBoundingRect = allUglyBoundingRect.intersected(
                          scale.mapRect(maxBoundsRect));
    QSizeF sizeF = allUglyBoundingRect.size();
    QPointF transF = allUglyBoundingRect.topLeft()/**resolution*/ -
            QPointF(qRound(allUglyBoundingRect.left()/**resolution*/),
                    qRound(allUglyBoundingRect.top()/**resolution*/));

    SkImageInfo info = SkImageInfo::Make(ceil(sizeF.width()),
                                         ceil(sizeF.height()),
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    //sk_sp<SkSurface> rasterSurface(SkSurface::MakeRaster(info));
    SkCanvas *rasterCanvas = new SkCanvas(bitmap);//rasterSurface->getCanvas();
    rasterCanvas->clear(SK_ColorTRANSPARENT);

    rasterCanvas->translate(-allUglyBoundingRect.left(),
                            -allUglyBoundingRect.top());

    allUglyBoundingRect.translate(-transF);

    rasterCanvas->translate(transF.x(), transF.y());

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
    rasterCanvas->flush();
    delete rasterCanvas;

    drawPos = SkPoint::Make(qRound(allUglyBoundingRect.left()),
                            qRound(allUglyBoundingRect.top()));

    if(!pixmapEffects.isEmpty()) {
        SkPixmap pixmap;
        bitmap.peekPixels(&pixmap);
        fmt_filters::image img((uint8_t*)pixmap.writable_addr(),
                               pixmap.width(), pixmap.height());
        foreach(PixmapEffectRenderData *effect, pixmapEffects) {
            effect->applyEffectsSk(bitmap, img, resolution);
        }
        clearPixmapEffects();
    }

    renderedImage = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
}
