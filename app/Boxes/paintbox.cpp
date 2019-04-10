#include "paintbox.h"
#include "canvas.h"
#include "MovablePoints/animatedpoint.h"
#include "PropertyUpdaters/paintboxsizeupdaters.h"
#include "Animators/transformanimator.h"

PaintBox::PaintBox() : BoundingBox(TYPE_PAINT) {
    setName("Paint Box");
}

void PaintBox::setupRenderData(
        const qreal &relFrame, BoundingBoxRenderData* data) {
    BoundingBox::setupRenderData(relFrame, data);
    auto paintData = GetAsSPtr(data, PaintBoxRenderData);
}

stdsptr<BoundingBoxRenderData> PaintBox::createRenderData() {
    return SPtrCreate(PaintBoxRenderData)(this);
}

void PaintBoxRenderData::drawSk(SkCanvas * const canvas) {
    //SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);

}
