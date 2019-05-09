#include "internallinkcanvas.h"
#include "linkcanvasrenderdata.h"
#include "Animators/transformanimator.h"
#include "canvas.h"

InternalLinkCanvas::InternalLinkCanvas(BoxesGroup * const linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    ca_prependChildAnimator(mTransformAnimator.data(), mClipToCanvas);
}

void InternalLinkCanvas::writeBoundingBox(QIODevice * const target) {
    InternalLinkGroupBox::writeBoundingBox(target);
    mClipToCanvas->writeProperty(target);
}

void InternalLinkCanvas::readBoundingBox(QIODevice * const target) {
    InternalLinkGroupBox::readBoundingBox(target);
    mClipToCanvas->readProperty(target);
}

void InternalLinkCanvas::setupRenderData(
        const qreal &relFrame, BoundingBoxRenderData * const data) {
    InternalLinkGroupBox::setupRenderData(relFrame, data);

    BoxesGroup* finalTarget = getFinalTarget();
    auto canvasData = GetAsSPtr(data, LinkCanvasRenderData);
    const auto canvasTarget = GetAsSPtr(finalTarget, Canvas);
    canvasData->fBgColor = toSkColor(canvasTarget->getBgColorAnimator()->
            getColorAtRelFrame(relFrame));
    //qreal res = getParentCanvas()->getResolutionFraction();
    canvasData->fCanvasHeight = canvasTarget->getCanvasHeight();//*res;
    canvasData->fCanvasWidth = canvasTarget->getCanvasWidth();//*res;
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
