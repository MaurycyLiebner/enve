#include "internallinkcanvas.h"
#include "linkcanvasrenderdata.h"
#include "Animators/transformanimator.h"
#include "canvas.h"

InternalLinkCanvas::InternalLinkCanvas(ContainerBox * const linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    ca_prependChildAnimator(mTransformAnimator.data(), mClipToCanvas);
}

void InternalLinkCanvas::setupRenderData(
        const qreal relFrame, BoundingBoxRenderData * const data) {
    InternalLinkGroupBox::setupRenderData(relFrame, data);

    ContainerBox* finalTarget = getFinalTarget();
    auto canvasData = GetAsSPtr(data, LinkCanvasRenderData);
    const auto canvasTarget = GetAsPtr(finalTarget, Canvas);
    canvasData->fBgColor = toSkColor(canvasTarget->getBgColorAnimator()->
            getColor(relFrame));
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
    if(isParentLink()) {
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

void InternalLinkCanvas::anim_setAbsFrame(const int frame) {
    InternalLinkGroupBox::anim_setAbsFrame(frame);
    const auto canvasTarget = GetAsPtr(getFinalTarget(), Canvas);
    if(!canvasTarget) return;
    canvasTarget->anim_setAbsFrame(anim_getCurrentRelFrame());
}
