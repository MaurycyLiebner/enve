// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "internallinkcanvas.h"
#include "linkcanvasrenderdata.h"
#include "Animators/transformanimator.h"
#include "canvas.h"

InternalLinkCanvas::InternalLinkCanvas(ContainerBox * const linkTarget,
                                       const bool innerLink) :
    InternalLinkGroupBox(linkTarget, innerLink) {
    mType = eBoxType::internalLinkCanvas;
    mFrameRemapping->disableAction();
    ca_prependChild(mTransformAnimator.data(), mClipToCanvas);
    ca_prependChild(mTransformAnimator.data(), mFrameRemapping);
}

void InternalLinkCanvas::enableFrameRemappingAction() {
    const auto finalTarget = static_cast<Canvas*>(getFinalTarget());
    const int minFrame = finalTarget->getMinFrame();
    const int maxFrame = finalTarget->getMaxFrame();
    mFrameRemapping->enableAction(minFrame, maxFrame, minFrame);
}

void InternalLinkCanvas::disableFrameRemappingAction() {
    mFrameRemapping->disableAction();
}

#include "typemenu.h"
#include <QInputDialog>
void InternalLinkCanvas::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<InternalLinkCanvas>()) return;
    menu->addedActionsForType<InternalLinkCanvas>();

    const PropertyMenu::CheckSelectedOp<InternalLinkCanvas> remapOp =
    [](InternalLinkCanvas * box, bool checked) {
        if(checked) box->enableFrameRemappingAction();
        else box->disableFrameRemappingAction();
    };
    menu->addCheckableAction("Frame Remapping",
                             mFrameRemapping->enabled(), remapOp);

    BoundingBox::setupCanvasMenu(menu);
}

void InternalLinkCanvas::setupRenderData(const qreal relFrame,
                                         BoxRenderData * const data,
                                         Canvas* const scene) {
    const qreal remapped = mFrameRemapping->frame(relFrame);
    InternalLinkGroupBox::setupRenderData(remapped, data, scene);
    data->fRelFrame = relFrame;

    ContainerBox* finalTarget = getFinalTarget();
    auto canvasData = static_cast<LinkCanvasRenderData*>(data);
    const auto canvasTarget = static_cast<Canvas*>(finalTarget);
    canvasData->fBgColor = toSkColor(canvasTarget->getBgColorAnimator()->
            getColor(relFrame));
    //qreal res = mParentScene->getResolution();
    canvasData->fCanvasHeight = canvasTarget->getCanvasHeight();//*res;
    canvasData->fCanvasWidth = canvasTarget->getCanvasWidth();//*res;
    if(getParentGroup()->isLink()) {
        const auto ilc = static_cast<InternalLinkCanvas*>(getLinkTarget());
        canvasData->fClipToCanvas = ilc->clipToCanvas();
    } else {
        canvasData->fClipToCanvas = mClipToCanvas->getValue();
    }
}

bool InternalLinkCanvas::clipToCanvas() {
    return mClipToCanvas->getValue();
}

qsptr<BoundingBox> InternalLinkCanvas::createLink(const bool inner) {
    auto linkBox = enve::make_shared<InternalLinkCanvas>(this, inner);
    copyTransformationTo(linkBox.get());
    return std::move(linkBox);
}

stdsptr<BoxRenderData> InternalLinkCanvas::createRenderData() {
    return enve::make_shared<LinkCanvasRenderData>(this);
}

bool InternalLinkCanvas::relPointInsidePath(const QPointF &relPos) const {
    if(mClipToCanvas->getValue()) return getRelBoundingRect().contains(relPos);
    return InternalLinkGroupBox::relPointInsidePath(relPos);
}

void InternalLinkCanvas::anim_setAbsFrame(const int frame) {
    InternalLinkGroupBox::anim_setAbsFrame(frame);
    const auto canvasTarget = static_cast<Canvas*>(getFinalTarget());
    if(!canvasTarget) return;
    canvasTarget->anim_setAbsFrame(anim_getCurrentRelFrame());
}
