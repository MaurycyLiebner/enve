#include "paintboxsizeupdaters.h"
#include "Boxes/paintbox.h"

PaintBoxSizeUpdaterBR::PaintBoxSizeUpdaterBR(PaintBox *paintBox) {
    mTarget = paintBox;
}

PaintBoxSizeUpdaterBR::~PaintBoxSizeUpdaterBR() {}

void PaintBoxSizeUpdaterBR::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PaintBoxSizeUpdaterBR::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterBR::finishedChange() {
    mTarget->scheduleFinishSizeSetup();
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

PaintBoxSizeUpdaterTL::PaintBoxSizeUpdaterTL(PaintBox *paintBox) {
    mTarget = paintBox;
}

PaintBoxSizeUpdaterTL::~PaintBoxSizeUpdaterTL() {}

void PaintBoxSizeUpdaterTL::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PaintBoxSizeUpdaterTL::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterTL::finishedChange() {
    mTarget->scheduleFinishSizeAndPosSetup();
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}
