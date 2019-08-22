#include "eboxorsound.h"

#include "canvas.h"
#include "Timeline/durationrectangle.h"

eBoxOrSound::eBoxOrSound(const QString &name) :
    StaticComplexAnimator(name) {
    connect(this, &eBoxOrSound::ancestorChanged, this, [this]() {
        mParentScene = mParentGroup ? mParentGroup->mParentScene : nullptr;
    });
    connect(this, &Property::prp_nameChanged, this,
            &SingleWidgetTarget::SWT_scheduleSearchContentUpdate);
}

void eBoxOrSound::setParentGroup(ContainerBox * const parent) {
    if(parent == mParentGroup) return;
    emit aboutToChangeParent();
    emit aboutToChangeAncestor();
    if(mParentGroup) {
        disconnect(mParentGroup, &eBoxOrSound::aboutToChangeAncestor,
                   this, &eBoxOrSound::aboutToChangeAncestor);
        disconnect(mParentGroup, &eBoxOrSound::ancestorChanged,
                   this, &eBoxOrSound::ancestorChanged);
    }
    prp_afterWholeInfluenceRangeChanged();
    mParentGroup = parent;
    mParent_k = parent;
    if(mParentGroup) {
        anim_setAbsFrame(mParentGroup->anim_getCurrentAbsFrame());
        connect(mParentGroup, &eBoxOrSound::aboutToChangeAncestor,
                this, &eBoxOrSound::aboutToChangeAncestor);
        connect(mParentGroup, &eBoxOrSound::ancestorChanged,
                this, &eBoxOrSound::ancestorChanged);
    }
    emit parentChanged(parent);
    emit ancestorChanged();
}

void eBoxOrSound::removeFromParent_k() {
    if(!mParentGroup) return;
    mParentGroup->removeContained_k(ref<eBoxOrSound>());
}

bool eBoxOrSound::isAncestor(const BoundingBox * const box) const {
    if(!mParentGroup) return false;
    if(mParentGroup == box) return true;
    if(box->SWT_isContainerBox()) return mParentGroup->isAncestor(box);
    return false;
}

bool eBoxOrSound::isFrameInDurationRect(const int relFrame) const {
    if(!mDurationRectangle) return true;
    return relFrame <= mDurationRectangle->getMaxRelFrame() &&
            relFrame >= mDurationRectangle->getMinRelFrame();
}

bool eBoxOrSound::isFrameFInDurationRect(const qreal relFrame) const {
    if(!mDurationRectangle) return true;
    return qCeil(relFrame) <= mDurationRectangle->getMaxRelFrame() &&
           qFloor(relFrame) >= mDurationRectangle->getMinRelFrame();
}

void eBoxOrSound::shiftAll(const int shift) {
    if(hasDurationRectangle()) mDurationRectangle->changeFramePosBy(shift);
    else anim_shiftAllKeys(shift);
}

QMimeData *eBoxOrSound::SWT_createMimeData() {
    return new eMimeData(QList<eBoxOrSound*>() << this);
}

FrameRange eBoxOrSound::prp_relInfluenceRange() const {
    if(mDurationRectangle) return mDurationRectangle->getRelFrameRange();
    return ComplexAnimator::prp_relInfluenceRange();
}

FrameRange eBoxOrSound::prp_getIdenticalRelRange(const int relFrame) const {
    if(mVisible) {
        const auto cRange = ComplexAnimator::prp_getIdenticalRelRange(relFrame);
        if(mDurationRectangle) {
            const auto dRange = mDurationRectangle->getRelFrameRange();
            if(relFrame > dRange.fMax) {
                return {mDurationRectangle->getMaxRelFrame() + 1,
                            FrameRange::EMAX};
            } else if(relFrame < dRange.fMin) {
                return {FrameRange::EMIN,
                        mDurationRectangle->getMinRelFrame() - 1};
            } else return cRange*dRange;
        }
        return cRange;
    }
    return {FrameRange::EMIN, FrameRange::EMAX};
}

int eBoxOrSound::prp_getRelFrameShift() const {
    if(!mDurationRectangle) return 0;
    return mDurationRectangle->getRelShift();
}

void eBoxOrSound::writeProperty(eWriteStream& dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst << mVisible;
    dst << mLocked;

    const bool hasDurRect = mDurationRectangle;
    dst << hasDurRect;
    if(hasDurRect) mDurationRectangle->writeDurationRectangle(dst);
}

void eBoxOrSound::readProperty(eReadStream& src) {
    StaticComplexAnimator::readProperty(src);
    src >> mVisible;
    src >> mLocked;

    bool hasDurRect;
    src >> hasDurRect;
    if(hasDurRect) {
        if(!mDurationRectangle) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(src);
        anim_shiftAllKeys(prp_getTotalFrameShift());
    }
}

TimelineMovable *eBoxOrSound::anim_getTimelineMovable(
        const int relX, const int minViewedFrame,
        const qreal pixelsPerFrame) {
    if(!mDurationRectangle) return nullptr;
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void eBoxOrSound::drawTimelineControls(QPainter * const p,
                                       const qreal pixelsPerFrame,
                                       const FrameRange &absFrameRange,
                                       const int rowHeight) {
    if(mDurationRectangle) {
        p->save();
        const int width = qCeil(absFrameRange.span()*pixelsPerFrame);
        const QRect drawRect(0, 0, width, rowHeight);
        const qreal fps = mParentScene ? mParentScene->getFps() : 1;
        mDurationRectangle->draw(p, drawRect, fps,
                                 pixelsPerFrame, absFrameRange);
        p->restore();
    }

    ComplexAnimator::drawTimelineControls(p, pixelsPerFrame,
                                          absFrameRange, rowHeight);
}

void eBoxOrSound::setDurationRectangle(
        const qsptr<DurationRectangle>& durationRect) {
    if(durationRect == mDurationRectangle) return;
    Q_ASSERT(!mDurationRectangleLocked);
    if(mDurationRectangle) {
        disconnect(mDurationRectangle.data(), nullptr, this, nullptr);
    }
    const FrameRange oldRange = mDurationRectangle ?
                mDurationRectangle->getAbsFrameRange() :
                FrameRange{FrameRange::EMIN, FrameRange::EMAX};
    const FrameRange newRange = durationRect ?
                durationRect->getAbsFrameRange() :
                FrameRange{FrameRange::EMIN, FrameRange::EMAX};
    const auto oldDurRect = mDurationRectangle;
    mDurationRectangle = durationRect;
    prp_afterChangedAbsRange(oldRange + newRange, false);
    if(!mDurationRectangle) return shiftAll(oldDurRect->getRelShift());

    connect(mDurationRectangle.data(), &DurationRectangle::shiftChanged,
            this, [this](const int oldShift, const int newShift) {
        const auto newRange = prp_absInfluenceRange();
        const auto oldRange = newRange.shifted(oldShift - newShift);
        prp_afterFrameShiftChanged(oldRange, newRange);
    });

    connect(mDurationRectangle.data(), &DurationRectangle::minFrameChanged,
            this, [this](const int oldMin, const int newMin) {
        const int min = qMin(newMin, oldMin);
        const int max = qMax(newMin, oldMin);
        prp_afterChangedAbsRange({min, max}, false);
    });
    connect(mDurationRectangle.data(), &DurationRectangle::maxFrameChanged,
            this, [this](const int oldMax, const int newMax) {
        const int min = qMin(newMax, oldMax);
        const int max = qMax(newMax, oldMax);
        prp_afterChangedAbsRange({min, max}, false);
    });
}

bool eBoxOrSound::isVisibleAndInVisibleDurationRect() const {
    return isFrameInDurationRect(anim_getCurrentRelFrame()) && mVisible;
}

bool eBoxOrSound::isVisibleAndInDurationRect(
        const int relFrame) const {
    return isFrameInDurationRect(relFrame) && mVisible;
}

bool eBoxOrSound::isFrameFVisibleAndInDurationRect(
        const qreal relFrame) const {
    return isFrameFInDurationRect(relFrame) && mVisible;
}

bool eBoxOrSound::hasDurationRectangle() const {
    return mDurationRectangle;
}

void eBoxOrSound::startDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startPosTransform();
    }
}

void eBoxOrSound::finishDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishPosTransform();
    }
}

void eBoxOrSound::moveDurationRect(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(dFrame);
    }
}

void eBoxOrSound::startMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMinFramePosTransform();
    }
}

void eBoxOrSound::finishMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMinFramePosTransform();
    }
}

void eBoxOrSound::moveMinFrame(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMinFrame(dFrame);
    }
}

void eBoxOrSound::startMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMaxFramePosTransform();
    }
}

void eBoxOrSound::finishMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMaxFramePosTransform();
    }
}

void eBoxOrSound::moveMaxFrame(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMaxFrame(dFrame);
    }
}

DurationRectangle *eBoxOrSound::getDurationRectangle() const {
    return mDurationRectangle.get();
}

void eBoxOrSound::createDurationRectangle() {
    const auto durRect = enve::make_shared<DurationRectangle>(*this);
//    durRect->setMinFrame(0);
//    if(mParentScene) durRect->setFramesDuration(mParentScene->getFrameCount());
    durRect->setMinRelFrame(anim_getCurrentRelFrame() - 5);
    durRect->setFramesDuration(10);
    setDurationRectangle(durRect);
}

void eBoxOrSound::setSelected(const bool select) {
    if(mSelected == select) return;
    mSelected = select;
    SWT_scheduleContentUpdate(SWT_BR_SELECTED);
    emit selectionChanged(select);
}

void eBoxOrSound::select() {
    setSelected(true);
}

void eBoxOrSound::deselect() {
    setSelected(false);
}

void eBoxOrSound::selectionChangeTriggered(const bool shiftPressed) {
    Q_ASSERT(mParentScene);
    if(!SWT_isBoundingBox()) return;
    const auto bb = static_cast<BoundingBox*>(this);
    if(shiftPressed) {
        if(mSelected) {
            mParentScene->removeBoxFromSelection(bb);
        } else {
            mParentScene->addBoxToSelection(bb);
        }
    } else {
        mParentScene->clearBoxesSelection();
        mParentScene->addBoxToSelection(bb);
    }
}

void eBoxOrSound::setVisibile(const bool visible) {
    if(mVisible == visible) return;
    mVisible = visible;

    prp_afterWholeInfluenceRangeChanged();

    SWT_scheduleContentUpdate(SWT_BR_VISIBLE);
    SWT_scheduleContentUpdate(SWT_BR_HIDDEN);

    emit visibilityChanged(visible);
}

void eBoxOrSound::switchVisible() {
    setVisibile(!mVisible);
}

void eBoxOrSound::switchLocked() {
    setLocked(!mLocked);
}

void eBoxOrSound::hide() {
    setVisibile(false);
}

void eBoxOrSound::show() {
    setVisibile(true);
}

bool eBoxOrSound::isVisible() const {
    return mVisible;
}

bool eBoxOrSound::isVisibleAndUnlocked() const {
    return isVisible() && !mLocked;
}

bool eBoxOrSound::isLocked() const {
    return mLocked;
}

void eBoxOrSound::lock() {
    setLocked(true);
}

void eBoxOrSound::unlock() {
    setLocked(false);
}

void eBoxOrSound::setLocked(const bool bt) {
    if(bt == mLocked) return;
    if(mParentScene && mSelected && SWT_isBoundingBox()) {
        const auto bb = static_cast<BoundingBox*>(this);
        mParentScene->removeBoxFromSelection(bb);
    }
    mLocked = bt;
    SWT_scheduleContentUpdate(SWT_BR_LOCKED);
    SWT_scheduleContentUpdate(SWT_BR_UNLOCKED);
}

void eBoxOrSound::moveUp() {
    mParentGroup->decreaseContainedZInList(this);
}

void eBoxOrSound::moveDown() {
    mParentGroup->increaseContainedZInList(this);
}

void eBoxOrSound::bringToFront() {
    mParentGroup->bringContainedToFrontList(this);
}

void eBoxOrSound::bringToEnd() {
    mParentGroup->bringContainedToEndList(this);
}
