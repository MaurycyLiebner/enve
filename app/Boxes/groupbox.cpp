#include "groupbox.h"
#include "durationrectangle.h"
#include "Animators/transformanimator.h"
#include "canvas.h"
#include "internallinkgroupbox.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "PropertyUpdaters/groupallpathsupdater.h"
#include "textbox.h"

GroupBox::GroupBox() : ContainerBox(TYPE_GROUP) {
    setName("Group");
}

qsptr<GroupBox> GroupBox::sReplaceLayerBox(const qsptr<LayerBox> &src) {
    const auto dst = SPtrCreate(GroupBox)();
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
void GroupBox::addActionsToMenu(BoxTypeMenu * const menu) {
    menu->addPlainAction<GroupBox>("Promote to Layer",
                                   [](GroupBox * box) {
        LayerBox::sReplaceGroupBox(GetAsSPtr(box, GroupBox));
    });
    ContainerBox::addActionsToMenu(menu);
}

void GroupBox::drawPixmapSk(SkCanvas * const canvas,
                            GrContext* const grContext) {
    for(const auto& box : mContainedBoxes) {
        if(box->isVisibleAndInVisibleDurationRect())
            box->drawPixmapSk(canvas, grContext);
    }
}

void GroupBox::setupRenderData(const qreal &relFrame,
                               BoundingBoxRenderData * const data) {
    Q_UNUSED(relFrame);
    data->fOpacity = 0;
}

stdsptr<BoundingBoxRenderData> GroupBox::createRenderData() {
    return SPtrCreate(GroupBoxRenderData)(this);
}
