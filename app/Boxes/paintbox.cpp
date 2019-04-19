#include "paintbox.h"
#include "canvas.h"
#include "MovablePoints/animatedpoint.h"
#include "Animators/transformanimator.h"

PaintBox::PaintBox() : BoundingBox(TYPE_PAINT) {
    setName("Paint Box");
    mSurface = SPtrCreate(AnimatedSurface)();
    ca_addChildAnimator(mSurface);
}

void PaintBox::setupRenderData(
        const qreal &relFrame, BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    auto paintData = GetAsSPtr(data, PaintBoxRenderData);
}

stdsptr<BoundingBoxRenderData> PaintBox::createRenderData() {
    return SPtrCreate(PaintBoxRenderData)(this);
}

#include <QFileDialog>
#include "typemenu.h"
void PaintBox::addActionsToMenu(BoxTypeMenu * const menu) {
    const auto widget = menu->getParentWidget();
    BoxTypeMenu::PlainOp<PaintBox> op = [widget](PaintBox * box) {
        const QString importPath = QFileDialog::getOpenFileName(
                                        widget,
                                        "Load From Image", "",
                                        "Image Files (*.png *.jpg)");
        if(!importPath.isEmpty()) {
            QImage img;
            if(img.load(importPath)) {
                //box->loadFromImage(img);
            }
        }
    };
    menu->addPlainAction("Load From Image", op);

    BoundingBox::addActionsToMenu(menu);
}

void PaintBoxRenderData::drawSk(SkCanvas * const canvas) {
    //SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);

}
