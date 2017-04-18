#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include "canvas.h"
#include "Boxes/vectorpath.h"
#include "movablepoint.h"
#include "Animators/qrealanimator.h"
#include "Animators/singlepathanimator.h"
#include <QDebug>

class UndoRedo
{
public:
    UndoRedo(QString name);
    virtual ~UndoRedo() {}
    virtual void undo() {}
    virtual void redo() {}
    void printName() { qDebug() << mName; }
    void printUndoName() { qDebug() << "UNDO " << mName; }
    void printRedoName() { qDebug() << "REDO " << mName; }

    int getFrame() { return mFrame; }
private:
    int mFrame;
    QString mName;
};

class UndoRedoSet : public UndoRedo
{
public:
    UndoRedoSet() : UndoRedo("UndoRedoSet") {
    }

    ~UndoRedoSet() {
        foreach(UndoRedo *undoRedo, mSet) {
            delete undoRedo;
        }
    }

    void undo() {
        for(int i = mSet.length() - 1; i >= 0; i--) {
            mSet.at(i)->undo();
        }
    }

    void redo() {
        foreach(UndoRedo* undoRedo, mSet) {
            undoRedo->redo();
        }
    }

    void addUndoRedo(UndoRedo* undoRedo) {
        mSet << undoRedo;
    }

    bool isEmpty() {
        return mSet.isEmpty();
    }

private:
    QList<UndoRedo*> mSet;
};


class UndoRedoStack {
public:
    UndoRedoStack() {
        startNewSet();
    }

    void startNewSet() {
        mNumberOfSets++;
    }

    ~UndoRedoStack() {
        clearUndoStack();
        clearRedoStack();
    }

    void setWindow(MainWindow *mainWindow);

    void finishSet() {
        mNumberOfSets--;
        if(mNumberOfSets == 0) {
            addSet();
        }
    }

    void addSet() {
        if((mCurrentSet == NULL) ? true : mCurrentSet->isEmpty()) {
            mCurrentSet = NULL;
            return;
        }
        addUndoRedo(mCurrentSet);
        mCurrentSet = NULL;
    }

    void addToSet(UndoRedo *undoRedo) {
        if(mCurrentSet == NULL) {
            mCurrentSet = new UndoRedoSet();
        }
        mCurrentSet->addUndoRedo(undoRedo);
    }

    void clearRedoStack() {
        foreach (UndoRedo *redoStackItem, mRedoStack) {
            delete redoStackItem;
        }
        mRedoStack.clear();
    }

    void clearUndoStack() {
        foreach (UndoRedo *undoStackItem, mUndoStack) {
            delete undoStackItem;
        }
        mUndoStack.clear();
    }

    void emptySomeOfUndo() {
        if(mUndoStack.length() > 150) {
//            for(int i = 0; i < 50; i++) {
                delete mUndoStack.takeFirst();
//            }
        }
    }

    void clearAll() {
        clearRedoStack();
        clearUndoStack();
    }

    void addUndoRedo(UndoRedo *undoRedo);

    void redo();

    void undo();

private:
    int mLastUndoRedoFrame = INT_MAX;
    MainWindow *mMainWindow;
    int mNumberOfSets = 0;
    UndoRedoSet *mCurrentSet = NULL;
    QList<UndoRedo*> mUndoStack;
    QList<UndoRedo*> mRedoStack;
};

class MoveMovablePointUndoRedo : public UndoRedo
{
public:
    MoveMovablePointUndoRedo(MovablePoint *movedPoint,
                         const QPointF &relPosBefore,
                         const QPointF &relPosAfter) :
        UndoRedo("MoveMovablePointUndoRedo") {
        mMovedPoint = movedPoint->ref<MovablePoint>();
        mRelPosAfter = relPosAfter;
        mRelPosBefore = relPosBefore;
    }

    ~MoveMovablePointUndoRedo() {
    }

    void redo() {
        mMovedPoint->setRelativePos(mRelPosAfter, false);
    }

    void undo() {
        mMovedPoint->setRelativePos(mRelPosBefore, false);
    }

private:
    QSharedPointer<MovablePoint> mMovedPoint;
    QPointF mRelPosBefore;
    QPointF mRelPosAfter;
};

class AppendToPointsListUndoRedo : public UndoRedo
{
public:
    AppendToPointsListUndoRedo(PathPoint *pointToAdd,
                               SinglePathAnimator *path) :
        UndoRedo("AppendToPointsListUndoRedo") {
        mPoint = pointToAdd->ref<PathPoint>();
        mPath = path->ref<SinglePathAnimator>();
    }

    ~AppendToPointsListUndoRedo() {
    }

    void redo() {
        mPath->appendToPointsList(mPoint.data(), false);
    }

    void undo() {
        mPath->removeFromPointsList(mPoint.data(), false);
    }

private:
    QSharedPointer<PathPoint> mPoint;
    QSharedPointer<SinglePathAnimator> mPath;
};

class RemoveFromPointsListUndoRedo : public AppendToPointsListUndoRedo
{
public:
    RemoveFromPointsListUndoRedo(PathPoint *pointToRemove,
                                 SinglePathAnimator *path) :
        AppendToPointsListUndoRedo(pointToRemove, path) {
    }

    void redo() {
        AppendToPointsListUndoRedo::undo();
    }

    void undo() {
        AppendToPointsListUndoRedo::redo();
    }
};

class SetNextPointUndoRedo : public UndoRedo
{
public:
    SetNextPointUndoRedo(PathPoint *point,
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

    ~SetNextPointUndoRedo() {
    }

    void redo(){
        mPoint->setNextPoint(mNewNext.data(), false);
    }

    void undo() {
        mPoint->setNextPoint(mOldNext.data(), false);
    }

private:
    QSharedPointer<PathPoint> mNewNext;
    QSharedPointer<PathPoint> mOldNext;
    QSharedPointer<PathPoint> mPoint;
};

class SetPreviousPointUndoRedo : public UndoRedo
{
public:
    SetPreviousPointUndoRedo(PathPoint *point,
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

    ~SetPreviousPointUndoRedo() {
    }

    void redo(){
        mPoint->setPreviousPoint(mNewPrev.data(), false);
    }

    void undo() {
        mPoint->setPreviousPoint(mOldPrev.data(), false);
    }

private:
    QSharedPointer<PathPoint> mNewPrev;
    QSharedPointer<PathPoint> mOldPrev;
    QSharedPointer<PathPoint> mPoint;
};

//class AddPointToSeparatePathsUndoRedo : public UndoRedo
//{
//public:
//    AddPointToSeparatePathsUndoRedo(PathAnimator *path,
//                                    PathPoint *point) :
//        UndoRedo("AddPointToSeparatePathsUndoRedo") {
//        mPath = path;
//        mPoint = point;
//    }

//    ~AddPointToSeparatePathsUndoRedo() {
//    }

//    void redo() {
//        mPath->addPointToSeparatePaths(mPoint, false);
//    }

//    void undo() {
//        mPath->removePointFromSeparatePaths(mPoint, false);
//    }

//private:
//    PathAnimator *mPath;
//    PathPoint *mPoint;
//};

//class RemovePointFromSeparatePathsUndoRedo :
//        public AddPointToSeparatePathsUndoRedo
//{
//public:
//    RemovePointFromSeparatePathsUndoRedo(PathAnimator *path,
//                                         PathPoint *point) :
//        AddPointToSeparatePathsUndoRedo(path, point) {

//    }

//    void redo() {
//        AddPointToSeparatePathsUndoRedo::undo();
//    }

//    void undo() {
//        AddPointToSeparatePathsUndoRedo::redo();
//    }
//};

class SetPathPointModeUndoRedo : public UndoRedo
{
public:
    SetPathPointModeUndoRedo(PathPoint *point,
                             const CtrlsMode &modeBefore,
                             const CtrlsMode &modeAfter) :
        UndoRedo("SetPathPointModeUndoRedo") {
        mPoint = point->ref<PathPoint>();
        mBefore = modeBefore;
        mAfter = modeAfter;
    }

    ~SetPathPointModeUndoRedo() {
    }

    void redo() {
        mPoint->setCtrlsMode(mAfter, false);
    }

    void undo() {
        mPoint->setCtrlsMode(mBefore, false);
    }

private:
    QSharedPointer<PathPoint> mPoint;
    CtrlsMode mBefore;
    CtrlsMode mAfter;
};

class SetCtrlPtEnabledUndoRedo : public UndoRedo
{
public:
    SetCtrlPtEnabledUndoRedo(const bool &enabled,
                             const bool &isStartPt,
                             PathPoint *parentPoint) :
        UndoRedo("SetCtrlPtEnabledUndoRedo") {
        mParentPoint = parentPoint;
        mEnabled = enabled;
        mIsStartPt = isStartPt;
    }

    ~SetCtrlPtEnabledUndoRedo() {
    }

    void redo() {
        mParentPoint->setCtrlPtEnabled(mEnabled, mIsStartPt, false);
    }

    void undo() {
        mParentPoint->setCtrlPtEnabled(!mEnabled, mIsStartPt, false);
    }

private:
    PathPoint *mParentPoint;
    bool mEnabled;
    bool mIsStartPt;
};

class MoveChildInListUndoRedo : public UndoRedo
{
public:
    MoveChildInListUndoRedo(BoundingBox *child,
                            const int &fromIndex,
                            const int &toIndex,
                            BoxesGroup *parentBox) :
        UndoRedo("MoveChildInListUndoRedo") {
        mParentBox = parentBox->ref<BoxesGroup>();
        mFromIndex = fromIndex;
        mToIndex = toIndex;
        mChild = child->ref<BoundingBox>();
    }

    ~MoveChildInListUndoRedo() {
    }

    void redo() {
        mParentBox->moveChildInList(mChild.data(),
                                    mFromIndex, mToIndex, false);
    }

    void undo() {
        mParentBox->moveChildInList(mChild.data(),
                                    mToIndex, mFromIndex, false);
    }
private:
    QSharedPointer<BoxesGroup> mParentBox;
    QSharedPointer<BoundingBox> mChild;
    int mFromIndex;
    int mToIndex;
};

class MoveChildAnimatorInListUndoRedo : public UndoRedo
{
public:
    MoveChildAnimatorInListUndoRedo(Property *child,
                            const int &fromIndex,
                            const int &toIndex,
                            ComplexAnimator *parentAnimator) :
        UndoRedo("MoveChildInListUndoRedo") {
        mParentAnimator = parentAnimator->ref<ComplexAnimator>();
        mFromIndex = fromIndex;
        mToIndex = toIndex;
        mChild = child->ref<Property>();
    }

    ~MoveChildAnimatorInListUndoRedo() {
    }

    void redo() {
        mParentAnimator->ca_moveChildInList(mChild.data(),
                                            mFromIndex, mToIndex, false);
    }

    void undo() {
        mParentAnimator->ca_moveChildInList(mChild.data(),
                                            mToIndex, mFromIndex, false);
    }
private:
    QSharedPointer<ComplexAnimator> mParentAnimator;
    QSharedPointer<Property> mChild;
    int mFromIndex;
    int mToIndex;
};

class SetBoundingBoxZListIndexUnoRedo : public UndoRedo
{
public:
    SetBoundingBoxZListIndexUnoRedo(const int &indexBefore,
                                    const int &indexAfter,
                                    BoundingBox *box) :
        UndoRedo("SetBoundingBoxZListIndexUnoRedo") {
        mBox = box->ref<BoundingBox>();
        mIndexAfter = indexAfter;
        mIndexBefore = indexBefore;
    }

    ~SetBoundingBoxZListIndexUnoRedo() {
    }

    void redo() {
        mBox->setZListIndex(mIndexAfter, false);
    }

    void undo() {
        mBox->setZListIndex(mIndexBefore, false);
    }

private:
    int mIndexBefore;
    int mIndexAfter;
    QSharedPointer<BoundingBox> mBox;
};

class AddChildToListUndoRedo : public UndoRedo
{
public:
    AddChildToListUndoRedo(BoxesGroup *parent,
                           const int &index,
                           BoundingBox *child) :
        UndoRedo("AddChildToListUndoRedo") {
        mParent = parent->ref<BoxesGroup>();
        mAddAtId = index;
        mChild = child->ref<BoundingBox>();
    }

    ~AddChildToListUndoRedo() {
    }

    void redo() {
        mParent->addChildToListAt(mAddAtId, mChild.data(), false);
    }

    void undo() {
        mParent->removeChildFromList(mAddAtId, false);
    }

private:
    QSharedPointer<BoxesGroup> mParent;
    int mAddAtId;
    QSharedPointer<BoundingBox> mChild;
};

class RemoveChildFromListUndoRedo : public AddChildToListUndoRedo
{
public:
    RemoveChildFromListUndoRedo(BoxesGroup *parent,
                                int index,
                                BoundingBox *child) :
        AddChildToListUndoRedo(parent, index, child) {

    }

    void redo() {
        AddChildToListUndoRedo::undo();
    }

    void undo() {
        AddChildToListUndoRedo::redo();
    }
private:
};

class SetBoxParentUndoRedo : public UndoRedo
{
public:
    SetBoxParentUndoRedo(BoundingBox *childBox,
                         BoxesGroup *oldParent,
                         BoxesGroup *newParent) :
        UndoRedo("SetBoxParentUndoRedo")
    {
        mChildBox = childBox->ref<BoundingBox>();
        mOldParent = oldParent->ref<BoxesGroup>();
        mNewParent = newParent->ref<BoxesGroup>();
    }

    ~SetBoxParentUndoRedo() {
    }

    void redo() {
        mChildBox->setParent(mNewParent.data(), false);
    }

    void undo() {
        mChildBox->setParent(mOldParent.data(), false);
    }

private:
    QSharedPointer<BoundingBox> mChildBox;
    QSharedPointer<BoxesGroup> mOldParent;
    QSharedPointer<BoxesGroup> mNewParent;
};

class SetPivotRelPosUndoRedo : public UndoRedo
{
public:
    SetPivotRelPosUndoRedo(BoundingBox *target,
                           const QPointF &prevRelPos,
                           const QPointF &newRelPos,
                           const bool &prevPivotChanged,
                           const bool &newPivotChanged) :
        UndoRedo("SetPivotRelPosUndoRedo") {
        mTarget = target->ref<BoundingBox>();
        mPrevRelPos = prevRelPos;
        mNewRelPos = newRelPos;
        mPrevPivotChanged = prevPivotChanged;
        mNewPivotChanged = newPivotChanged;
    }

    ~SetPivotRelPosUndoRedo() {
    }

    void redo() {
        mTarget->setPivotRelPos(mNewRelPos, false, mNewPivotChanged);
    }

    void undo() {
        mTarget->setPivotRelPos(mPrevRelPos, false, mPrevPivotChanged);
    }

private:
    QSharedPointer<BoundingBox> mTarget;
    QPointF mPrevRelPos;
    QPointF mNewRelPos;
    bool mPrevPivotChanged;
    bool mNewPivotChanged;
};

class  SetBoxVisibleUndoRedo : public UndoRedo
{
public:
     SetBoxVisibleUndoRedo(BoundingBox *target,
                           const bool &visibleBefore,
                           const bool &visibleAfter) :
         UndoRedo("SetBoxVisibleUndoRedo") {
         mTarget = target->ref<BoundingBox>();
         mVisibleAfter = visibleAfter;
         mVisibleBefore = visibleBefore;
     }

     ~SetBoxVisibleUndoRedo() {
     }

     void redo() {
         mTarget->setVisibile(mVisibleAfter, false);
     }

     void undo() {
         mTarget->setVisibile(mVisibleBefore, false);
     }

private:
     bool mVisibleBefore;
     bool mVisibleAfter;
     QSharedPointer<BoundingBox> mTarget;
};

class ChangeQrealAnimatorValue : public UndoRedo
{
public:
    ChangeQrealAnimatorValue(const qreal &oldValue,
                             const qreal &newValue,
                             QrealAnimator *animator) :
        UndoRedo("ChangeQrealAnimatorValue") {
        mOldValue = oldValue;
        mNewValue = newValue;
        mAnimator = animator->ref<QrealAnimator>();
    }

    ~ChangeQrealAnimatorValue() {
    }

    void redo() {
        mAnimator->qra_setCurrentValue(mNewValue);
    }

    void undo() {
        mAnimator->qra_setCurrentValue(mOldValue);
    }

private:
    qreal mOldValue;
    qreal mNewValue;
    QSharedPointer<QrealAnimator> mAnimator;
};

class ChangeQrealKeyValueUndoRedo : public UndoRedo
{
public:
    ChangeQrealKeyValueUndoRedo(const qreal &oldValue,
                                const qreal &newValue,
                                QrealKey *key) :
        UndoRedo("ChangeQrealKeyValueUndoRedo") {
        mOldValue = oldValue;
        mNewValue = newValue;
        mTargetKey = key->ref<QrealKey>();
    }

    ~ChangeQrealKeyValueUndoRedo() {
    }

    void redo() {
        ((QrealAnimator*)mTargetKey->getParentAnimator())->
                qra_saveValueToKey(mTargetKey.get(), mNewValue, false);
    }

    void undo() {
        ((QrealAnimator*)mTargetKey->getParentAnimator())->
                qra_saveValueToKey(mTargetKey.get(), mOldValue, false);
    }

private:
    qreal mOldValue;
    qreal mNewValue;
    std::shared_ptr<QrealKey> mTargetKey;
};

class ChangeKeyFrameUndoRedo : public UndoRedo
{
public:
    ChangeKeyFrameUndoRedo(const int &oldFrame,
                           const int &newFrame,
                           Key *key) :
        UndoRedo("ChangeKeyFrameUndoRedo") {
        mOldFrame = oldFrame;
        mNewFrame = newFrame;
        mTargetKey = key->ref<Key>();
    }

    ~ChangeKeyFrameUndoRedo() {
    }

    void redo() {
        mTargetKey->getParentAnimator()->anim_moveKeyToFrame(mTargetKey.get(),
                                                             mNewFrame);
    }

    void undo() {
        mTargetKey->getParentAnimator()->anim_moveKeyToFrame(mTargetKey.get(),
                                                             mOldFrame);
    }

private:
    int mOldFrame;
    int mNewFrame;
    std::shared_ptr<Key> mTargetKey;
};

class AnimatorRecordingSetUndoRedo : public UndoRedo
{
public:
    AnimatorRecordingSetUndoRedo(const bool &recordingOld,
                                 const bool &recordingNew,
                                 Animator *animator) :
    UndoRedo("QrealAnimatorRecordingSetUndoRedo") {
        mRecordingOld = recordingOld;
        mRecordingNew = recordingNew;
        mAnimator = animator->ref<Animator>();
    }

    ~AnimatorRecordingSetUndoRedo() {
    }

    void undo() {
        mAnimator->anim_setRecordingWithoutChangingKeys(mRecordingOld, false);
    }

    void redo() {
        mAnimator->anim_setRecordingWithoutChangingKeys(mRecordingNew, false);
    }

private:
    bool mRecordingOld;
    bool mRecordingNew;
    QSharedPointer<Animator> mAnimator;
};

class AddKeyToAnimatorUndoRedo : public UndoRedo
{
public:
    AddKeyToAnimatorUndoRedo(Key *key, Animator *animator) :
        UndoRedo("AddQrealKeyToAnimatorUndoRedo") {
        mKey = key->ref<Key>();
        mAnimator = animator->ref<Animator>();
    }

    ~AddKeyToAnimatorUndoRedo() {
    }

    void redo() {
        mAnimator->anim_appendKey(mKey.get(), false);
    }

    void undo() {
        mAnimator->anim_removeKey(mKey.get(), false);
    }

private:
    std::shared_ptr<Key> mKey;
    QSharedPointer<Animator> mAnimator;
};

class RemoveKeyFromAnimatorUndoRedo :
        public AddKeyToAnimatorUndoRedo
{
public:
    RemoveKeyFromAnimatorUndoRedo(Key *key, Animator *animator) :
        AddKeyToAnimatorUndoRedo(key, animator) {

    }

    void redo() {
        AddKeyToAnimatorUndoRedo::undo();
    }

    void undo() {
        AddKeyToAnimatorUndoRedo::redo();
    }
};

#include "Animators/paintsettings.h"

class PaintTypeChangeUndoRedo : public UndoRedo {
public:
    PaintTypeChangeUndoRedo(const PaintType &oldType,
                            const PaintType &newType,
                            PaintSettings *target) :
        UndoRedo("PaintTypeChangeUndoRedo") {
        mOldType = oldType;
        mNewType = newType;
        mTarget = target->ref<PaintSettings>();
    }

    ~PaintTypeChangeUndoRedo() {
    }

    void redo() {
        mTarget->setPaintType(mNewType, false);
    }

    void undo() {
        mTarget->setPaintType(mOldType, false);
    }

private:
    PaintType mOldType;
    PaintType mNewType;
    QSharedPointer<PaintSettings> mTarget;
};

class GradientChangeUndoRedo : public UndoRedo {
public:
    GradientChangeUndoRedo(Gradient *oldGradient,
                           Gradient *newGradient,
                           PaintSettings *target) :
        UndoRedo("GradientChangeUndoRedo") {
        mTarget = target->ref<PaintSettings>();
        mOldGradient = oldGradient->ref<Gradient>();
        mNewGradient = newGradient->ref<Gradient>();
    }

    ~GradientChangeUndoRedo() {
    }

    void redo() {
        mTarget->setGradient(mNewGradient.data(), false);
    }

    void undo() {
        mTarget->setGradient(mOldGradient.data(), false);
    }

private:
    QSharedPointer<Gradient> mOldGradient;
    QSharedPointer<Gradient> mNewGradient;
    QSharedPointer<PaintSettings> mTarget;
};


class GradientColorAddedToListUndoRedo : public UndoRedo {
public:
    GradientColorAddedToListUndoRedo(Gradient *target,
                                     ColorAnimator *color) :
        UndoRedo("GradientColorAddedToListUndoRedo") {
        mGradient = target->ref<Gradient>();
        mColor = color->ref<ColorAnimator>();
    }

    ~GradientColorAddedToListUndoRedo() {
    }

    void undo() {
        mGradient->removeColor(mColor.data(), false);
    }

    void redo() {
        mGradient->addColorToList(mColor.data(), false);
    }

private:
    QSharedPointer<ColorAnimator> mColor;
    QSharedPointer<Gradient> mGradient;
};

class GradientColorRemovedFromListUndoRedo :
        public GradientColorAddedToListUndoRedo {
public:
    GradientColorRemovedFromListUndoRedo(Gradient *target,
                                     ColorAnimator *color) :
        GradientColorAddedToListUndoRedo(target,
                                         color) {
    }

    void undo() {
        GradientColorAddedToListUndoRedo::redo();
    }

    void redo() {
        GradientColorAddedToListUndoRedo::undo();
    }
};

class GradientSwapColorsUndoRedo : public UndoRedo {
public:
    GradientSwapColorsUndoRedo(Gradient *target,
                               const int &id1,
                               const int &id2) :
        UndoRedo("GradientSwapColorsUndoRedo") {
        mGradient = target->ref<Gradient>();
        mId1 = id1;
        mId2 = id2;
    }

    ~GradientSwapColorsUndoRedo() {
    }

    void undo() {
        mGradient->swapColors(mId2, mId1, false);
    }

    void redo() {
        mGradient->swapColors(mId1, mId2, false);
    }

private:
    int mId1;
    int mId2;
    QSharedPointer<Gradient> mGradient;
};

class AddSinglePathAnimatorUndoRedo : public UndoRedo {
public:
    AddSinglePathAnimatorUndoRedo(PathAnimator *target,
                                  SinglePathAnimator *path) :
        UndoRedo("AddSinglePathAnimatorUndoRedo") {
        mTarget = target->ref<PathAnimator>();
        mPath = path->ref<SinglePathAnimator>();
    }

    ~AddSinglePathAnimatorUndoRedo() {

    }

    void undo() {
        mTarget->removeSinglePathAnimator(mPath.data(), false);
    }

    void redo() {
        mTarget->addSinglePathAnimator(mPath.data(), false);
    }
private:
    QSharedPointer<PathAnimator> mTarget;
    QSharedPointer<SinglePathAnimator> mPath;
};

class RemoveSinglePathAnimatorUndoRedo :
        public AddSinglePathAnimatorUndoRedo {
public:
    RemoveSinglePathAnimatorUndoRedo(PathAnimator *target,
                                     SinglePathAnimator *path) :
        AddSinglePathAnimatorUndoRedo(target, path) {
    }

    void undo() {
        AddSinglePathAnimatorUndoRedo::redo();
    }

    void redo() {
        AddSinglePathAnimatorUndoRedo::undo();
    }
};

class ChangeSinglePathFirstPoint :
        public UndoRedo {
public:
    ChangeSinglePathFirstPoint(SinglePathAnimator *target,
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

    ~ChangeSinglePathFirstPoint() {

    }

    void undo() {
        mTarget->replaceSeparatePathPoint(mOldPoint.data(), false);
    }

    void redo() {
        mTarget->replaceSeparatePathPoint(mNewPoint.data(), false);
    }

private:
    QSharedPointer<SinglePathAnimator> mTarget;
    QSharedPointer<PathPoint> mOldPoint;
    QSharedPointer<PathPoint> mNewPoint;
};

class ReversePointsDirectionUndoRedo :
        public UndoRedo {
public:
    ReversePointsDirectionUndoRedo(PathPoint *target) :
        UndoRedo("ReversePointsDirectionUndoRedo") {
        mTarget = target->ref<PathPoint>();
    }

    ~ReversePointsDirectionUndoRedo() {
    }

    void undo() {
        mTarget->reversePointsDirectionReverse();
    }

    void redo() {
        mTarget->reversePointsDirection();
    }
private:
    QSharedPointer<PathPoint> mTarget;
};

#endif // UNDOREDO_H
