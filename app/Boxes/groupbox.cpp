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
