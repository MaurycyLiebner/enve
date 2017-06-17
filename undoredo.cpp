#include "undoredo.h"
#include "mainwindow.h"
#include "qrealkey.h"
#include "canvas.h"
#include "Boxes/vectorpath.h"
#include "movablepoint.h"
#include "Animators/qrealanimator.h"
#include "Animators/singlepathanimator.h"
#include "Animators/paintsettings.h"
#include "pathpoint.h"
#include "Animators/pathanimator.h"

void UndoRedoStack::setWindow(MainWindow *mainWindow) {
    mMainWindow = mainWindow;
}

void UndoRedoStack::addUndoRedo(UndoRedo *undoRedo) {
    mMainWindow->setFileChangedSinceSaving(true);
    if(mNumberOfSets != 0) {
        addToSet(undoRedo);
    } else {
//        int currentFrame = mMainWindow->getCurrentFrame();
//        if(currentFrame != mLastUndoRedoFrame) {
//            UndoRedo *frameChangeUndoRedo =
//                    new ChangeViewedFrameUndoRedo(mLastUndoRedoFrame,
//                                                  currentFrame);
//            mLastUndoRedoFrame = currentFrame;
//            addUndoRedo(frameChangeUndoRedo);
//        }
//        if(currentFrame != mLastUndoRedoFrame) {
//            bool noSet = mCurrentSet == NULL;

//            if(noSet) {
//                addToSet(undoRedo);
//                addToSet(new ChangeViewedFrameUndoRedo(mLastUndoRedoFrame, currentFrame));
//                mLastUndoRedoFrame = currentFrame;
//                addSet();
//                return;
//            } else {
//                addToSet(new ChangeViewedFrameUndoRedo(mLastUndoRedoFrame, currentFrame));
//                mLastUndoRedoFrame = currentFrame;
//            }
//        }
        clearRedoStack();
        emptySomeOfUndo();
        mUndoStack << undoRedo;
    }
}

void UndoRedoStack::redo() {
    if(mRedoStack.isEmpty()) {
        return;
    }
    UndoRedo *toRedo = mRedoStack.last();
    if(toRedo->getFrame() != mMainWindow->getCurrentFrame()) {
        mMainWindow->setCurrentFrameForAllWidgets(toRedo->getFrame());
        return;
    }
    mRedoStack.removeLast();
    toRedo->printRedoName();
    toRedo->redo();
    mUndoStack << toRedo;
}

void UndoRedoStack::undo() {
    if(mUndoStack.isEmpty()) {
        return;
    }
    UndoRedo *toUndo = mUndoStack.last();
    if(toUndo->getFrame() != mMainWindow->getCurrentFrame()) {
        mMainWindow->setCurrentFrameForAllWidgets(toUndo->getFrame());
        return;
    }
    mUndoStack.removeLast();
    toUndo->printUndoName();
    toUndo->undo();
    mRedoStack << toUndo;
}

UndoRedo::UndoRedo(QString name) {
    mName = name;
    printName();
    mFrame = MainWindow::getInstance()->getCurrentFrame();
}

ChangeQrealKeyValueUndoRedo::ChangeQrealKeyValueUndoRedo(const SkScalar &oldValue,
                                                         const SkScalar &newValue,
                                                         QrealKey *key) :
    UndoRedo("ChangeQrealKeyValueUndoRedo") {
    mOldValue = oldValue;
    mNewValue = newValue;
    mTargetKey = key->ref<QrealKey>();
}

ChangeQrealKeyValueUndoRedo::~ChangeQrealKeyValueUndoRedo() {
}

void ChangeQrealKeyValueUndoRedo::redo() {
    ((QrealAnimator*)mTargetKey->getParentAnimator())->
            qra_saveValueToKey(mTargetKey.get(), mNewValue, false, true);
}

void ChangeQrealKeyValueUndoRedo::undo() {
    ((QrealAnimator*)mTargetKey->getParentAnimator())->
            qra_saveValueToKey(mTargetKey.get(), mOldValue, false, true);
}

AppendToPointsListUndoRedo::AppendToPointsListUndoRedo(
        PathPoint *pointToAdd, SinglePathAnimator *path) :
    UndoRedo("AppendToPointsListUndoRedo") {
    mPoint = pointToAdd->ref<PathPoint>();
    mPath = path->ref<SinglePathAnimator>();
}

AppendToPointsListUndoRedo::~AppendToPointsListUndoRedo() {
}

void AppendToPointsListUndoRedo::redo() {
    mPath->appendToPointsList(mPoint.data(), false);
}

void AppendToPointsListUndoRedo::undo() {
    mPath->removeFromPointsList(mPoint.data(), false);
}

RemoveFromPointsListUndoRedo::RemoveFromPointsListUndoRedo(
        PathPoint *pointToRemove, SinglePathAnimator *path) :
    AppendToPointsListUndoRedo(pointToRemove, path) {
}

void RemoveFromPointsListUndoRedo::redo() {
    AppendToPointsListUndoRedo::undo();
}

void RemoveFromPointsListUndoRedo::undo() {
    AppendToPointsListUndoRedo::redo();
}

SetNextPointUndoRedo::SetNextPointUndoRedo(PathPoint *point,
                                           PathPoint *oldNext,
                                           PathPoint *newNext) :
    UndoRedo("SetNextPointUndoRedo") {
    if(newNext != NULL) {
        mNewNext = newNext->ref<PathPoint>();
    }
    if(oldNext != NULL) {
        mOldNext = oldNext->ref<PathPoint>();
    }
    mPoint = point->ref<PathPoint>();
}

SetNextPointUndoRedo::~SetNextPointUndoRedo() {
}

void SetNextPointUndoRedo::redo(){
    mPoint->setNextPoint(mNewNext.data(), false);
}

void SetNextPointUndoRedo::undo() {
    mPoint->setNextPoint(mOldNext.data(), false);
}

SetPreviousPointUndoRedo::SetPreviousPointUndoRedo(PathPoint *point,
                                                   PathPoint *oldPrevious,
                                                   PathPoint *newPrevious) :
    UndoRedo("SetPreviousPointUndoRedo") {
    if(newPrevious != NULL) {
        mNewPrev = newPrevious->ref<PathPoint>();
    }
    if(oldPrevious != NULL) {
        mOldPrev = oldPrevious->ref<PathPoint>();
    }
    mPoint = point->ref<PathPoint>();
}

SetPreviousPointUndoRedo::~SetPreviousPointUndoRedo() {
}

void SetPreviousPointUndoRedo::redo(){
    mPoint->setPreviousPoint(mNewPrev.data(), false);
}

void SetPreviousPointUndoRedo::undo() {
    mPoint->setPreviousPoint(mOldPrev.data(), false);
}

SetPathPointModeUndoRedo::SetPathPointModeUndoRedo(PathPoint *point,
                                                   const CtrlsMode &modeBefore,
                                                   const CtrlsMode &modeAfter) :
    UndoRedo("SetPathPointModeUndoRedo") {
    mPoint = point->ref<PathPoint>();
    mBefore = modeBefore;
    mAfter = modeAfter;
}

SetPathPointModeUndoRedo::~SetPathPointModeUndoRedo() {
}

void SetPathPointModeUndoRedo::redo() {
    mPoint->setCtrlsMode(mAfter, false);
}

void SetPathPointModeUndoRedo::undo() {
    mPoint->setCtrlsMode(mBefore, false);
}

SetCtrlPtEnabledUndoRedo::SetCtrlPtEnabledUndoRedo(const bool &enabled,
                                                   const bool &isStartPt,
                                                   PathPoint *parentPoint) :
    UndoRedo("SetCtrlPtEnabledUndoRedo") {
    mParentPoint = parentPoint;
    mEnabled = enabled;
    mIsStartPt = isStartPt;
}

SetCtrlPtEnabledUndoRedo::~SetCtrlPtEnabledUndoRedo() {
}

void SetCtrlPtEnabledUndoRedo::redo() {
    mParentPoint->setCtrlPtEnabled(mEnabled, mIsStartPt, false);
}

void SetCtrlPtEnabledUndoRedo::undo() {
    mParentPoint->setCtrlPtEnabled(!mEnabled, mIsStartPt, false);
}

MoveChildInListUndoRedo::MoveChildInListUndoRedo(BoundingBox *child,
                                                 const int &fromIndex,
                                                 const int &toIndex,
                                                 BoxesGroup *parentBox) :
    UndoRedo("MoveChildInListUndoRedo") {
    mParentBox = parentBox->ref<BoxesGroup>();
    mFromIndex = fromIndex;
    mToIndex = toIndex;
    mChild = child->ref<BoundingBox>();
}

MoveChildInListUndoRedo::~MoveChildInListUndoRedo() {
}

void MoveChildInListUndoRedo::redo() {
    mParentBox->moveChildInList(mChild.data(),
                                mFromIndex, mToIndex, false);
}

void MoveChildInListUndoRedo::undo() {
    mParentBox->moveChildInList(mChild.data(),
                                mToIndex, mFromIndex, false);
}

MoveChildAnimatorInListUndoRedo::MoveChildAnimatorInListUndoRedo(
                                    Property *child,
                                    const int &fromIndex,
                                    const int &toIndex,
                                    ComplexAnimator *parentAnimator) :
    UndoRedo("MoveChildInListUndoRedo") {
    mParentAnimator = parentAnimator->ref<ComplexAnimator>();
    mFromIndex = fromIndex;
    mToIndex = toIndex;
    mChild = child->ref<Property>();
}

MoveChildAnimatorInListUndoRedo::~MoveChildAnimatorInListUndoRedo() {
}

void MoveChildAnimatorInListUndoRedo::redo() {
    mParentAnimator->ca_moveChildInList(mChild.data(),
                                        mFromIndex, mToIndex, false);
}

void MoveChildAnimatorInListUndoRedo::undo() {
    mParentAnimator->ca_moveChildInList(mChild.data(),
                                        mToIndex, mFromIndex, false);
}

SetBoundingBoxZListIndexUnoRedo::SetBoundingBoxZListIndexUnoRedo(
                                    const int &indexBefore,
                                    const int &indexAfter,
                                    BoundingBox *box) :
    UndoRedo("SetBoundingBoxZListIndexUnoRedo") {
    mBox = box->ref<BoundingBox>();
    mIndexAfter = indexAfter;
    mIndexBefore = indexBefore;
}

SetBoundingBoxZListIndexUnoRedo::~SetBoundingBoxZListIndexUnoRedo() {
}

void SetBoundingBoxZListIndexUnoRedo::redo() {
    mBox->setZListIndex(mIndexAfter, false);
}

void SetBoundingBoxZListIndexUnoRedo::undo() {
    mBox->setZListIndex(mIndexBefore, false);
}

AddChildToListUndoRedo::AddChildToListUndoRedo(BoxesGroup *parent,
                                               const int &index,
                                               BoundingBox *child) :
    UndoRedo("AddChildToListUndoRedo") {
    mParent = parent->ref<BoxesGroup>();
    mAddAtId = index;
    mChild = child->ref<BoundingBox>();
}

AddChildToListUndoRedo::~AddChildToListUndoRedo() {
}

void AddChildToListUndoRedo::redo() {
    mParent->addChildToListAt(mAddAtId, mChild.data(), false);
}

void AddChildToListUndoRedo::undo() {
    mParent->removeChildFromList(mAddAtId, false);
}

SetBoxParentUndoRedo::SetBoxParentUndoRedo(BoundingBox *childBox,
                                           BoxesGroup *oldParent,
                                           BoxesGroup *newParent) :
    UndoRedo("SetBoxParentUndoRedo")
{
    mChildBox = childBox->ref<BoundingBox>();
    mOldParent = oldParent->ref<BoxesGroup>();
    mNewParent = newParent->ref<BoxesGroup>();
}

SetBoxParentUndoRedo::~SetBoxParentUndoRedo() {
}

void SetBoxParentUndoRedo::redo() {
    mChildBox->setParent(mNewParent.data(), false);
}

void SetBoxParentUndoRedo::undo() {
    mChildBox->setParent(mOldParent.data(), false);
}

SetBoxVisibleUndoRedo::SetBoxVisibleUndoRedo(BoundingBox *target,
                                             const bool &visibleBefore,
                                             const bool &visibleAfter) :
    UndoRedo("SetBoxVisibleUndoRedo") {
    mTarget = target->ref<BoundingBox>();
    mVisibleAfter = visibleAfter;
    mVisibleBefore = visibleBefore;
}

SetBoxVisibleUndoRedo::~SetBoxVisibleUndoRedo() {
}

void SetBoxVisibleUndoRedo::redo() {
    mTarget->setVisibile(mVisibleAfter, false);
}

void SetBoxVisibleUndoRedo::undo() {
    mTarget->setVisibile(mVisibleBefore, false);
}

ChangeQrealAnimatorValue::ChangeQrealAnimatorValue(const SkScalar &oldValue,
                                                   const SkScalar &newValue,
                                                   QrealAnimator *animator) :
    UndoRedo("ChangeQrealAnimatorValue") {
    mOldValue = oldValue;
    mNewValue = newValue;
    mAnimator = animator->ref<QrealAnimator>();
}

ChangeQrealAnimatorValue::~ChangeQrealAnimatorValue() {
}

void ChangeQrealAnimatorValue::redo() {
    mAnimator->qra_setCurrentValue(mNewValue, false, true);
}

void ChangeQrealAnimatorValue::undo() {
    mAnimator->qra_setCurrentValue(mOldValue, false, true);
}

ChangeKeyFrameUndoRedo::ChangeKeyFrameUndoRedo(const int &oldFrame,
                                               const int &newFrame,
                                               Key *key) :
    UndoRedo("ChangeKeyFrameUndoRedo") {
    mOldFrame = oldFrame;
    mNewFrame = newFrame;
    mTargetKey = key->ref<Key>();
}

ChangeKeyFrameUndoRedo::~ChangeKeyFrameUndoRedo() {
}

void ChangeKeyFrameUndoRedo::redo() {
    mTargetKey->getParentAnimator()->anim_moveKeyToRelFrame(mTargetKey.get(),
                                                            mNewFrame,
                                                            false,
                                                            true);
}

void ChangeKeyFrameUndoRedo::undo() {
    mTargetKey->getParentAnimator()->anim_moveKeyToRelFrame(mTargetKey.get(),
                                                            mOldFrame,
                                                            false,
                                                            true);
}

AnimatorRecordingSetUndoRedo::AnimatorRecordingSetUndoRedo(
        const bool &recordingOld,
        const bool &recordingNew,
        Animator *animator) :
    UndoRedo("QrealAnimatorRecordingSetUndoRedo") {
    mRecordingOld = recordingOld;
    mRecordingNew = recordingNew;
    mAnimator = animator->ref<Animator>();
}

AnimatorRecordingSetUndoRedo::~AnimatorRecordingSetUndoRedo() {
}

void AnimatorRecordingSetUndoRedo::undo() {
    mAnimator->anim_setRecordingWithoutChangingKeys(mRecordingOld, false);
}

void AnimatorRecordingSetUndoRedo::redo() {
    mAnimator->anim_setRecordingWithoutChangingKeys(mRecordingNew, false);
}

AddKeyToAnimatorUndoRedo::AddKeyToAnimatorUndoRedo(Key *key,
                                                   Animator *animator) :
    UndoRedo("AddQrealKeyToAnimatorUndoRedo") {
    mKey = key->ref<Key>();
    mAnimator = animator->ref<Animator>();
}

AddKeyToAnimatorUndoRedo::~AddKeyToAnimatorUndoRedo() {
}

void AddKeyToAnimatorUndoRedo::redo() {
    mAnimator->anim_appendKey(mKey.get(), false);
}

void AddKeyToAnimatorUndoRedo::undo() {
    mAnimator->anim_removeKey(mKey.get(), false);
}

PaintTypeChangeUndoRedo::PaintTypeChangeUndoRedo(const PaintType &oldType,
                                                 const PaintType &newType,
                                                 PaintSettings *target) :
    UndoRedo("PaintTypeChangeUndoRedo") {
    mOldType = oldType;
    mNewType = newType;
    mTarget = target->ref<PaintSettings>();
}

PaintTypeChangeUndoRedo::~PaintTypeChangeUndoRedo() {
}

void PaintTypeChangeUndoRedo::redo() {
    mTarget->setPaintType(mNewType, false);
}

void PaintTypeChangeUndoRedo::undo() {
    mTarget->setPaintType(mOldType, false);
}

GradientChangeUndoRedo::GradientChangeUndoRedo(Gradient *oldGradient,
                                               Gradient *newGradient,
                                               PaintSettings *target) :
    UndoRedo("GradientChangeUndoRedo") {
    mTarget = target->ref<PaintSettings>();
    if(oldGradient != NULL) {
        mOldGradient = oldGradient->ref<Gradient>();
    }
    if(newGradient != NULL) {
        mNewGradient = newGradient->ref<Gradient>();
    }
}

GradientChangeUndoRedo::~GradientChangeUndoRedo() {
}

void GradientChangeUndoRedo::redo() {
    mTarget->setGradient(mNewGradient.data(), false);
}

void GradientChangeUndoRedo::undo() {
    mTarget->setGradient(mOldGradient.data(), false);
}

GradientColorAddedToListUndoRedo::GradientColorAddedToListUndoRedo(
        Gradient *target, ColorAnimator *color) :
    UndoRedo("GradientColorAddedToListUndoRedo") {
    mGradient = target->ref<Gradient>();
    mColor = color->ref<ColorAnimator>();
}

GradientColorAddedToListUndoRedo::~GradientColorAddedToListUndoRedo() {
}

void GradientColorAddedToListUndoRedo::undo() {
    mGradient->removeColor(mColor.data(), false);
}

void GradientColorAddedToListUndoRedo::redo() {
    mGradient->addColorToList(mColor.data(), false);
}

GradientSwapColorsUndoRedo::GradientSwapColorsUndoRedo(Gradient *target,
                                                       const int &id1,
                                                       const int &id2) :
    UndoRedo("GradientSwapColorsUndoRedo") {
    mGradient = target->ref<Gradient>();
    mId1 = id1;
    mId2 = id2;
}

GradientSwapColorsUndoRedo::~GradientSwapColorsUndoRedo() {
}

void GradientSwapColorsUndoRedo::undo() {
    mGradient->swapColors(mId2, mId1, false);
}

void GradientSwapColorsUndoRedo::redo() {
    mGradient->swapColors(mId1, mId2, false);
}

AddSinglePathAnimatorUndoRedo::AddSinglePathAnimatorUndoRedo(
        PathAnimator *target, SinglePathAnimator *path) :
    UndoRedo("AddSinglePathAnimatorUndoRedo") {
    mTarget = target->ref<PathAnimator>();
    mPath = path->ref<SinglePathAnimator>();
}

AddSinglePathAnimatorUndoRedo::~AddSinglePathAnimatorUndoRedo() {

}

void AddSinglePathAnimatorUndoRedo::undo() {
    mTarget->removeSinglePathAnimator(mPath.data(), false);
}

void AddSinglePathAnimatorUndoRedo::redo() {
    mTarget->addSinglePathAnimator(mPath.data(), false);
}

ChangeSinglePathFirstPoint::ChangeSinglePathFirstPoint(
        SinglePathAnimator *target,
        PathPoint *oldPoint,
        PathPoint *newPoint) :
    UndoRedo("ChangeSinglePathFirstPoint") {
    mTarget = target->ref<SinglePathAnimator>();
    if(oldPoint != NULL) {
        mOldPoint = oldPoint->ref<PathPoint>();
    }
    if(newPoint != NULL) {
        mNewPoint = newPoint->ref<PathPoint>();
    }
}

ChangeSinglePathFirstPoint::~ChangeSinglePathFirstPoint() {

}

void ChangeSinglePathFirstPoint::undo() {
    mTarget->replaceSeparatePathPoint(mOldPoint.data(), false);
}

void ChangeSinglePathFirstPoint::redo() {
    mTarget->replaceSeparatePathPoint(mNewPoint.data(), false);
}

ReversePointsDirectionUndoRedo::ReversePointsDirectionUndoRedo(
        PathPoint *target) :
    UndoRedo("ReversePointsDirectionUndoRedo") {
    mTarget = target->ref<PathPoint>();
}

ReversePointsDirectionUndoRedo::~ReversePointsDirectionUndoRedo() {
}

void ReversePointsDirectionUndoRedo::undo() {
    mTarget->reversePointsDirectionReverse();
}

void ReversePointsDirectionUndoRedo::redo() {
    mTarget->reversePointsDirection();
}
#include "Boxes/textbox.h"
ChangeFontUndoRedo::ChangeFontUndoRedo(TextBox *target,
                                       const QFont &fontBefore,
                                       const QFont &fontAfter) :
    UndoRedo("ChangeFontUndoRedo") {
    mTarget = target->ref<TextBox>();
    mOldFont = fontBefore;
    mNewFont = fontAfter;
}

ChangeFontUndoRedo::~ChangeFontUndoRedo() {

}

void ChangeFontUndoRedo::undo() {
    mTarget->setFont(mOldFont, false);
}

void ChangeFontUndoRedo::redo() {
    mTarget->setFont(mNewFont, false);
}

ChangeTextUndoRedo::ChangeTextUndoRedo(TextBox *target,
                                       const QString &textBefore,
                                       const QString &textAfter) :
    UndoRedo("ChangeTextUndoRedo") {
    mTarget = target->ref<TextBox>();
    mOldText = textBefore;
    mNewText = textAfter;
}

ChangeTextUndoRedo::~ChangeTextUndoRedo() {
}

void ChangeTextUndoRedo::undo() {
    mTarget->setText(mOldText, false);
}

void ChangeTextUndoRedo::redo() {
    mTarget->setText(mNewText, false);
}
