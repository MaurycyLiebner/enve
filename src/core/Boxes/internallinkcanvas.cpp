#include "internallinkcanvas.h"
#include "linkcanvasrenderdata.h"
#include "Animators/transformanimator.h"
#include "canvas.h"

InternalLinkCanvas::InternalLinkCanvas(ContainerBox * const linkTarget) :
    InternalLinkGroupBox(linkTarget) {
    ca_prependChildAnimator(mTransformAnimator.data(), mClipToCanvas);
}

void InternalLinkCanvas::setupRenderData(
        const qreal relFrame, BoxRenderData * const data) {
    InternalLinkGroupBox::setupRenderData(relFrame, data);

    ContainerBox* finalTarget = getFinalTarget();
    auto canvasData = data->ref<LinkCanvasRenderData>();
    const auto canvasTarget = static_cast<Canvas*>(finalTarget);
    canvasData->fBgColor = toSkColor(canvasTarget->getBgColorAnimator()->
            getColor(relFrame));
    //qreal res = mParentScene->getResolutionFraction();
    canvasData->fCanvasHeight = canvasTarget->getCanvasHeight();//*res;
    canvasData->fCanvasWidth = canvasTarget->getCanvasWidth();//*res;
    if(mParentGroup->SWT_isLinkBox()) {
        const auto ilc = static_cast<InternalLinkCanvas*>(getLinkTarget());
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
        return enve::make_shared<InternalLinkCanvas>(this);
    }
}

stdsptr<BoxRenderData> InternalLinkCanvas::createRenderData() {
    return enve::make_shared<LinkCanvasRenderData>(this);
}

bool InternalLinkCanvas::relPointInsidePath(const QPointF &relPos) const {
    if(mClipToCanvas->getValue()) return mRelRect.contains(relPos);
    return InternalLinkGroupBox::relPointInsidePath(relPos);
}

void InternalLinkCanvas::anim_setAbsFrame(const int frame) {
    InternalLinkGroupBox::anim_setAbsFrame(frame);
    const auto canvasTarget = static_cast<Canvas*>(getFinalTarget());
    if(!canvasTarget) return;
    canvasTarget->anim_setAbsFrame(anim_getCurrentRelFrame());
}
