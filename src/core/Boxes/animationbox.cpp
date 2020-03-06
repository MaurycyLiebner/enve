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

#include "animationbox.h"

#include <QMenu>
#include <QInputDialog>

#include "canvas.h"
#include "FileCacheHandlers/animationcachehandler.h"
#include "imagebox.h"
#include "undoredo.h"
#include "Animators/qrealkey.h"
#include "RasterEffects/rastereffectcollection.h"
#include "frameremapping.h"
#include "typemenu.h"
#include "Private/Tasks/complextask.h"
#include "Private/Tasks/taskscheduler.h"
#include "GUI/dialogsinterface.h"

AnimationBox::AnimationBox(const QString &name, const eBoxType type) :
    BoundingBox(name, type) {
    connect(this, &eBoxOrSound::parentChanged,
            this, &AnimationBox::updateAnimationRange);

    setDurationRectangle(enve::make_shared<FixedLenAnimationRect>(*this), true);

    mFrameRemapping = enve::make_shared<IntFrameRemapping>();
    ca_prependChild(mRasterEffectsAnimators.get(), mFrameRemapping);
    mFrameRemapping->disableAction();

    connect(mFrameRemapping.get(), &IntFrameRemapping::enabledChanged,
            this, &AnimationBox::updateAnimationRange);
}

FixedLenAnimationRect *AnimationBox::getAnimationDurationRect() const {
    return static_cast<FixedLenAnimationRect*>(getDurationRectangle());
}

void AnimationBox::updateAnimationRange() {
    const auto durRect = getAnimationDurationRect();
    if(mFrameRemapping->enabled()) {
        const int nFrames = durRect->getFrameDuration();
        durRect->setAnimationFrameDuration(0);
        durRect->setFramesDuration(nFrames);
    } else {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 0;
        const int nFrames = qRound(frameCount*qAbs(mStretch));
        durRect->setAnimationFrameDuration(nFrames);
    }
}

void AnimationBox::animationDataChanged() {
    updateAnimationRange();
    if(mFrameRemapping->enabled()) {
        int frameCount;
        if(mSrcFramesCache) frameCount = mSrcFramesCache->getFrameCount();
        else frameCount = 1;
        mFrameRemapping->setFrameCount(frameCount);
    }
    prp_afterWholeInfluenceRangeChanged();
}

bool AnimationBox::shouldScheduleUpdate() {
    if(!mSrcFramesCache) return false;
    return BoundingBox::shouldScheduleUpdate();
}

int AnimationBox::getAnimationFrameForRelFrame(const qreal relFrame) {
    const int lastFrameId = mSrcFramesCache->getFrameCount() - 1;
    const int animStartRelFrame =
                getAnimationDurationRect()->getMinAnimRelFrame();
    int pixId;
    if(mFrameRemapping->enabled()) {
        pixId = mFrameRemapping->frame(relFrame);
    } else {
        if(isZero6Dec(mStretch)) {
            pixId = lastFrameId;
        } else {
            pixId = qRound((relFrame - animStartRelFrame)/qAbs(mStretch));
            if(mStretch < 0) pixId = lastFrameId - pixId;
        }
    }

    if(pixId < 0) pixId = 0;
    else if(pixId > lastFrameId) pixId = lastFrameId;

    return pixId;
}

void AnimationBox::enableFrameRemappingAction() {
    const int frameCount = mSrcFramesCache->getFrameCount();
    const auto durRect = getAnimationDurationRect();
    const int animStartRelFrame = durRect->getMinAnimRelFrame();
    mFrameRemapping->enableAction(0, frameCount - 1, animStartRelFrame);
}

void AnimationBox::disableFrameRemappingAction() {
    mFrameRemapping->disableAction();
}

void AnimationBox::reload() {
    if(mSrcFramesCache) mSrcFramesCache->reload();
}

void AnimationBox::setAnimationFramesHandler(const qsptr<AnimationFrameHandler>& src) {
    mSrcFramesCache = src;
}

void AnimationBox::anim_setAbsFrame(const int frame) {
    BoundingBox::anim_setAbsFrame(frame);
    if(!mSrcFramesCache) return;
    planUpdate(UpdateReason::frameChange);
}

FrameRange AnimationBox::prp_getIdenticalRelRange(const int relFrame) const {
    if(isVisibleAndInDurationRect(relFrame) && !mFrameRemapping->enabled()) {
        const auto animDur = getAnimationDurationRect();
        const auto animRange = animDur->getAnimRelRange();
        if(animRange.inRange(relFrame))
            return {relFrame, relFrame};
        else if(relFrame > animRange.fMax) {
            const auto baseRange = BoundingBox::prp_getIdenticalRelRange(relFrame);
            const FrameRange durRect{animRange.fMax + 1,
                                     animDur->getRelFrameRange().fMax};
            return baseRange*durRect;
        } else if(relFrame < animRange.fMin) {
            const auto baseRange = BoundingBox::prp_getIdenticalRelRange(relFrame);
            const FrameRange durRect{animDur->getRelFrameRange().fMin,
                                     animRange.fMin - 1};
            return baseRange*durRect;
        }
    }
    return BoundingBox::prp_getIdenticalRelRange(relFrame);
}

class AnimationToPaint : public QObject {
public:
    using Loader = std::function<void(int, int)>;
    using Cancel = std::function<void()>;
    AnimationToPaint(const int firstAbsFrame, const int firstRelFrame,
                     const int iMax, const int increment,
                     AnimationBox* const box,
                     AnimationFrameHandler* const src,
                     ComplexTask* const cplxTask,
                     const Loader& loader,
                     const Cancel& cancel,
                     QObject* const parent) :
        QObject(parent),
        mFirstAbsFrame(firstAbsFrame), mFirstRelFrame(firstRelFrame),
        mIMax(iMax), mIncrement(increment),
        mBox(box), mSrc(src), mCplxTask(cplxTask),
        mLoader(loader), mCancel(cancel) {}

    void process() {
        if(!mCplxTask) return;
        if(!mBox || !mSrc) return mCplxTask->cancel();
        if(mI > mIMax) {
            const int finishValue = mCplxTask->finishValue();
            mCplxTask->setValue(finishValue);
            return;
        };
        const int relFrame = mFirstRelFrame + mI;
        const int absFrame = mFirstAbsFrame + mI;
        const int animFrame = mBox->getAnimationFrameForRelFrame(relFrame);
        auto task = mSrc->scheduleFrameLoad(animFrame);
        if(!task) {
            const auto emptyTask = enve::make_shared<eCustomCpuTask>(
                        nullptr, nullptr, nullptr, nullptr);
            emptyTask->queTask();
            task = emptyTask.get();
        }
        mCplxTask->addTask(task->ref<eTask>());
        const QPointer<AnimationToPaint> ptr = this;
        const QPointer<ComplexTask> cplxPtr = mCplxTask;
        const int i = mI;
        task->addDependent({[ptr, cplxPtr, animFrame, absFrame, i]() {
            if(!ptr) return;
            ptr->mLoader(animFrame, absFrame);
            ptr->process();
            if(cplxPtr) {
                const int newValue = qMax(cplxPtr->value(), i);
                cplxPtr->setValue(newValue);
            }
        }, mCancel});
        mI += mIncrement;
    }
private:
    const int mFirstAbsFrame;
    const int mFirstRelFrame;
    const int mIMax;
    const int mIncrement;

    const QPointer<AnimationBox> mBox;
    const QPointer<AnimationFrameHandler> mSrc;
    const QPointer<ComplexTask> mCplxTask;
    const Loader mLoader;
    const std::function<void()> mCancel;

    int mI = 0;
};

void AnimationBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<AnimationBox>()) return;
    menu->addedActionsForType<AnimationBox>();
    const auto widget = menu->getParentWidget();

    const PropertyMenu::PlainSelectedOp<AnimationBox> reloadOp =
    [](AnimationBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const PropertyMenu::PlainSelectedOp<AnimationBox> setSrcOp =
    [](AnimationBox * box) {
        box->changeSourceFile();
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    const PropertyMenu::CheckSelectedOp<AnimationBox> remapOp =
    [](AnimationBox * box, bool checked) {
        if(checked) box->enableFrameRemappingAction();
        else box->disableFrameRemappingAction();
    };
    menu->addCheckableAction("Frame Remapping",
                             mFrameRemapping->enabled(), remapOp);

    const PropertyMenu::PlainSelectedOp<AnimationBox> stretchOp =
    [widget](AnimationBox * box) {
        bool ok = false;
        const int stretch = QInputDialog::getInt(widget,
                                                 "Stretch " + box->prp_getName(),
                                                 "Stretch:",
                                                 qRound(box->getStretch()*100),
                                                 -1000, 1000, 1, &ok);
        if(!ok) return;
        box->setStretch(stretch*0.01);
    };
    menu->addPlainAction("Stretch...", stretchOp);

    menu->addSeparator();
    const PropertyMenu::PlainSelectedOp<AnimationBox> createPaintObj =
    [widget](AnimationBox * box) {
        int firstAbsFrame;
        int lastAbsFrame;
        int increment;
        const auto& instance = DialogsInterface::instance();
        const bool ok = instance.execAnimationToPaint(
                    box, firstAbsFrame, lastAbsFrame, increment, widget);
        if(ok) box->createPaintObject(firstAbsFrame, lastAbsFrame, increment);
    };
    menu->addPlainAction("Create Paint Object...", createPaintObj);

    BoundingBox::setupCanvasMenu(menu);
}

void AnimationBox::createPaintObject(const int firstAbsFrame,
                                     const int lastAbsFrame,
                                     const int increment) {
    const qptr<ContainerBox> parent = getParentGroup();
    if(!parent) return;
    const auto paintObj = enve::make_shared<PaintBox>();
    copyBoundingBoxDataTo(paintObj.get());
    const auto surface = paintObj->getSurface();
    surface->anim_setAbsFrame(firstAbsFrame);
    surface->anim_setRecording(true);
    const auto src = mSrcFramesCache.get();
    const auto loader = [src, surface](const int animFrame, const int absFrame) {
        const auto cont = src->getFrameAtFrame(animFrame);
        if(!cont) return;
        surface->anim_setAbsFrame(absFrame);
        surface->loadPixmap(cont->getImage());
    };
    const auto adder = [paintObj, parent]() {
        if(!parent) return;
        parent->addContained(paintObj);
    };
    const int firstRelFrame = prp_absFrameToRelFrame(firstAbsFrame);
    const int iMax = lastAbsFrame - firstAbsFrame;
    const auto complexTask = QSharedPointer<ComplexTask>(
                new ComplexTask(iMax, "Video to Paint"));
    const auto cancel = [complexTask]() {
        complexTask->cancel();
    };

    const auto taskCreator = new AnimationToPaint(
                firstAbsFrame, firstRelFrame, iMax, increment,
                this, src, complexTask.get(), loader, cancel,
                complexTask.get());

    taskCreator->process();

    if(complexTask->done()) {
        adder();
    } else {
        connect(complexTask.get(), &ComplexTask::finishedAll,
                paintObj.get(), adder);
        TaskScheduler::sInstance->addComplexTask(complexTask);
    }
}

void AnimationBox::setupRenderData(const qreal relFrame,
                                   BoxRenderData * const data,
                                   Canvas* const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
    const auto imgData = static_cast<AnimationBoxRenderData*>(data);
    const int animFrame = getAnimationFrameForRelFrame(relFrame);
    imgData->fAnimFrame = animFrame;
    const auto upd = mSrcFramesCache->scheduleFrameLoad(animFrame);
    if(upd) upd->addDependent(imgData);
    else {
        const auto cont = mSrcFramesCache->getFrameAtFrame(animFrame);
        imgData->setContainer(cont);
    }
}

stdsptr<BoxRenderData> AnimationBox::createRenderData() {
    return enve::make_shared<AnimationBoxRenderData>(mSrcFramesCache.get(), this);
}

AnimationBoxRenderData::AnimationBoxRenderData(AnimationFrameHandler *cacheHandler, BoundingBox *parentBox) :
    ImageContainerRenderData(parentBox),
    fSrcCacheHandler(cacheHandler) {}

void AnimationBoxRenderData::loadImageFromHandler() {
    if(!fSrcCacheHandler) return;
    const auto cont = fSrcCacheHandler->getFrameAtOrBeforeFrame(fAnimFrame);
    setContainer(cont);
}
