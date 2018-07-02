#include "property.h"
#include "Animators/complexanimator.h"
#include "Animators/animatorupdater.h"
#include "mainwindow.h"

Property::Property() {
    mMainWindow = MainWindow::getInstance();
}

void Property::prp_valueChanged() {
    prp_updateInfluenceRangeAfterChanged();
}

void Property::prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                   const int &maxFrame) {
    emit prp_absFrameRangeChanged(minFrame, maxFrame);
}

const QString &Property::prp_getName() {
    return prp_mName;
}

int Property::prp_getFrameShift() const {
    return prp_getRelFrameShift() + prp_getParentFrameShift();
}

int Property::prp_getParentFrameShift() const {
    return prp_mParentFrameShift;
}

void Property::prp_setParentFrameShift(const int &shift,
                                       ComplexAnimator *parentAnimator) {
    Q_UNUSED(parentAnimator);
    prp_mParentFrameShift = shift;
}

int Property::prp_absFrameToRelFrame(const int &absFrame) const {
    if(absFrame == INT_MIN) return INT_MIN;
    if(absFrame == INT_MAX) return INT_MAX;
    return absFrame - prp_getFrameShift();
}

qreal Property::prp_absFrameToRelFrameF(const qreal &absFrame) const {
    if(qRound(absFrame) == INT_MIN) return INT_MIN;
    if(qRound(absFrame) == INT_MAX) return INT_MAX;
    return absFrame - prp_getFrameShift();
}

int Property::prp_relFrameToAbsFrame(const int &relFrame) const {
    if(relFrame == INT_MIN) return INT_MIN;
    if(relFrame == INT_MAX) return INT_MAX;
    return relFrame + prp_getFrameShift();
}

qreal Property::prp_relFrameToAbsFrameF(const qreal &relFrame) const {
    if(qRound(relFrame) == INT_MIN) return INT_MIN;
    if(qRound(relFrame) == INT_MAX) return INT_MAX;
    return relFrame + prp_getFrameShift();
}

void Property::prp_setName(const QString &newName) {
    prp_mName = newName;
}

void Property::prp_setUpdater(AnimatorUpdater *updater) {
    if(prp_mUpdaterBlocked) return;
    if(updater == nullptr) {
        prp_mUpdater.reset();
    } else {
        prp_mUpdater = updater->ref<AnimatorUpdater>();
    }
}

void Property::prp_setBlockedUpdater(AnimatorUpdater *updater) {
    prp_mUpdaterBlocked = false;
    prp_setUpdater(updater);
    prp_blockUpdater();
}

void Property::prp_updateInfluenceRangeAfterChanged() {
    emit prp_updateWholeInfluenceRange();
}

void Property::prp_blockUpdater() {
    prp_mUpdaterBlocked = true;
}

void Property::prp_callUpdater() {
    if(prp_mUpdater.get() == nullptr) {
        return;
    } else {
        prp_mUpdater->update();
    }
}

void Property::prp_callFinishUpdater() {
    if(prp_mUpdater.get() == nullptr) return;
    prp_mUpdater->updateFinal();
}

void Property::addUndoRedo(UndoRedo *undoRedo) {
    UndoRedoStack *stack = mMainWindow->getUndoRedoStack();
    if(stack == nullptr) return;
    stack->addUndoRedo(undoRedo);
}

void Property::callUpdateSchedulers() {
    mMainWindow->callUpdateSchedulers();
}

MainWindow *Property::getMainWindow() {
    return mMainWindow;
}

bool Property::isShiftPressed() {
    return mMainWindow->isShiftPressed();
}

bool Property::isShiftPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ShiftModifier;
}

bool Property::isCtrlPressed() {
    return mMainWindow->isCtrlPressed();
}

bool Property::isCtrlPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::ControlModifier;
}

bool Property::isAltPressed() {
    return mMainWindow->isAltPressed();
}

bool Property::isAltPressed(QKeyEvent *event) {
    return event->modifiers() & Qt::AltModifier;
}

int Property::getCurrentFrameFromMainWindow() {
    return mMainWindow->getCurrentFrame();
}

int Property::getFrameCount() {
    return mMainWindow->getFrameCount();
}

void Property::graphUpdateAfterKeysChanged()
{
    //mMainWindow->getKeysView()->graphUpdateAfterKeysChanged();
}

void Property::graphScheduleUpdateAfterKeysChanged() {
    //mMainWindow->getKeysView()->scheduleGraphUpdateAfterKeysChanged();
}

void Property::schedulePivotUpdate() {
    mMainWindow->schedulePivotUpdate();
}
