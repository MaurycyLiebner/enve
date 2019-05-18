#include "Boxes/layerbox.h"
#include "undoredo.h"
#include <QApplication>
#include "GUI/mainwindow.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "textbox.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"
#include "GUI/canvaswindow.h"
#include "canvas.h"
#include "Boxes/particlebox.h"
#include "durationrectangle.h"
#include "linkbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "Animators/transformanimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Boxes/internallinkgroupbox.h"

bool LayerBox::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

LayerBox::LayerBox(const BoundingBoxType &type) :
    GroupBox(type) {
    setName("Layer");
}

LayerBox::~LayerBox() {}

qsptr<BoundingBox> LayerBox::createLink() {
    const auto linkBox = SPtrCreate(InternalLinkGroupBox)(this);
    copyBoundingBoxDataTo(linkBox.get());
    return std::move(linkBox);
}

void processChildData(BoundingBox* child,
                      LayerBoxRenderData* parentData,
                      const qreal& boxRelFrame) {
    auto boxRenderData =
            GetAsSPtr(child->getCurrentRenderData(qRound(boxRelFrame)),
                      BoundingBoxRenderData);
    if(boxRenderData) {
        if(boxRenderData->fCopied) {
            child->nullifyCurrentRenderData(boxRenderData->fRelFrame);
        }
    } else {
        boxRenderData = child->createRenderData();
        boxRenderData->fReason = parentData->fReason;
        //boxRenderData->parentIsTarget = false;
        boxRenderData->fUseCustomRelFrame = true;
        boxRenderData->fCustomRelFrame = boxRelFrame;
        boxRenderData->scheduleTask();
    }
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;
}

void LayerBox::setupRenderData(const qreal &relFrame,
                                 BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto groupData = GetAsPtr(data, LayerBoxRenderData);
    groupData->fChildrenRenderData.clear();
    groupData->fOtherGlobalRects.clear();
    qreal childrenEffectsMargin = 0;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    for(const auto& box : mContainedBoxes) {
        const qreal boxRelFrame = box->prp_absFrameToRelFrameF(absFrame);
        if(box->isFrameFVisibleAndInDurationRect(boxRelFrame)) {
            processChildData(box.data(), groupData, boxRelFrame);

            childrenEffectsMargin =
                    qMax(box->getEffectsMarginAtRelFrameF(boxRelFrame),
                         childrenEffectsMargin);
        }
    }

    data->fEffectsMargin += childrenEffectsMargin;
}


void LayerBox::drawPixmapSk(SkCanvas * const canvas,
                              GrContext* const grContext) {
    if(shouldPaintOnImage()) {
        BoundingBox::drawPixmapSk(canvas, grContext);
    } else {
        SkPaint paint;
        const int intAlpha = qRound(mTransformAnimator->getOpacity()*2.55);
        paint.setAlpha(static_cast<U8CPU>(intAlpha));
        paint.setBlendMode(mBlendModeSk);
        canvas->saveLayer(nullptr, &paint);
        for(const auto& box : mContainedBoxes) {
            //box->draw(p);
            if(box->isVisibleAndInVisibleDurationRect())
                box->drawPixmapSk(canvas, grContext);
        }
        canvas->restore();
    }
}

#include "Animators/gpueffectanimators.h"
bool LayerBox::shouldPaintOnImage() const {
    if(SWT_isLinkBox() || SWT_isCanvas()) return true;
    if(mIsDescendantCurrentGroup) return false;
//    return !mIsDescendantCurrentGroup; !!!
    return mEffectsAnimators->hasEffects() ||
           mGPUEffectsAnimators->hasEffects();
}

bool LayerBox::SWT_isLayerBox() const { return true; }

void LayerBox::anim_setAbsFrame(const int &frame) {
    BoundingBox::anim_setAbsFrame(frame);

    updateDrawRenderContainerTransform();
    for(const auto& box : mContainedBoxes) {
        box->anim_setAbsFrame(frame);
    }
}

stdsptr<BoundingBoxRenderData> LayerBox::createRenderData() {
    return SPtrCreate(LayerBoxRenderData)(this);
}
