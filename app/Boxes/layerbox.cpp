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
#include "Boxes/groupbox.h"

bool LayerBox::mCtrlsAlwaysVisible = false;

//bool zMoreThan(BoundingBox *box1, BoundingBox *box2)
//{
//    return box1->getZIndex() < box2->getZIndex();
//}

LayerBox::LayerBox(const BoundingBoxType &type) : ContainerBox(type) {
    setName("Layer");
}

qsptr<LayerBox> LayerBox::sReplaceGroupBox(const qsptr<GroupBox> &src) {
    const auto dst = SPtrCreate(LayerBox)();
    src->copyBoundingBoxDataTo(dst.get());
    const auto children = src->getContainedBoxesList();
    const auto srcParent = src->getParentGroup();
    if(srcParent) srcParent->replaceContainedBox(src, dst);
    for(const auto& child : children) {
        src->removeContainedBox_k(child);
        dst->addContainedBox(child);
    }
    return dst;
}

#include "typemenu.h"
void LayerBox::addActionsToMenu(BoxTypeMenu * const menu) {
    menu->addPlainAction<LayerBox>("Demote to Group",
                                   [](LayerBox * box) {
        GroupBox::sReplaceLayerBox(GetAsSPtr(box, LayerBox));
    });
    ContainerBox::addActionsToMenu(menu);
}

qsptr<BoundingBox> LayerBox::createLink() {
    const auto linkBox = SPtrCreate(InternalLinkGroupBox)(this);
    copyBoundingBoxDataTo(linkBox.get());
    return std::move(linkBox);
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
    return mEffectsAnimators->hasEffects() ||
           mGPUEffectsAnimators->hasEffects();
}

#include "groupbox.h"
void processChildData(BoundingBox * const child,
                      LayerBoxRenderData * const parentData,
                      const qreal& childRelFrame,
                      const qreal& absFrame,
                      qreal& childrenEffectsMargin) {
    if(!child->isFrameFVisibleAndInDurationRect(childRelFrame)) return;
    if(child->SWT_isGroupBox()) {
        const auto childGroup = GetAsPtr(child, GroupBox);
        const auto descs = childGroup->getContainedBoxesList();
        for(const auto& desc : descs) {
            processChildData(desc.get(), parentData,
                             desc->prp_absFrameToRelFrameF(absFrame),
                             absFrame, childrenEffectsMargin);
        }
        return;
    }
    auto boxRenderData =
            GetAsSPtr(child->getCurrentRenderData(qRound(childRelFrame)),
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
        boxRenderData->fCustomRelFrame = childRelFrame;
        boxRenderData->scheduleTask();
    }
    boxRenderData->addDependent(parentData);
    parentData->fChildrenRenderData << boxRenderData;

    childrenEffectsMargin =
            qMax(child->getEffectsMarginAtRelFrameF(childRelFrame),
                 childrenEffectsMargin);
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
        processChildData(box.data(), groupData, boxRelFrame,
                         absFrame, childrenEffectsMargin);
    }

    data->fEffectsMargin += childrenEffectsMargin;
}

stdsptr<BoundingBoxRenderData> LayerBox::createRenderData() {
    return SPtrCreate(LayerBoxRenderData)(this);
}
