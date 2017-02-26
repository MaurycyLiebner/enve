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
                         QPointF relPosBefore,
                         QPointF relPosAfter) :
        UndoRedo("MoveMovablePointUndoRedo") {
        mMovedPoint = movedPoint;
        mMovedPoint->incNumberPointers();
        mRelPosAfter = relPosAfter;
        mRelPosBefore = relPosBefore;
    }

    ~MoveMovablePointUndoRedo() {
        mMovedPoint->decNumberPointers();
    }

    void redo() {
        mMovedPoint->setRelativePos(mRelPosAfter, false);
    }

    void undo() {
        mMovedPoint->setRelativePos(mRelPosBefore, false);
    }

private:
    MovablePoint *mMovedPoint;
    QPointF mRelPosBefore;
    QPointF mRelPosAfter;
};

class AppendToPointsListUndoRedo : public UndoRedo
{
public:
    AppendToPointsListUndoRedo(PathPoint *pointToAdd,
                               SinglePathAnimator *path) :
        UndoRedo("AppendToPointsListUndoRedo") {
        mPoint = pointToAdd;
        mPath = path;
        mPoint->incNumberPointers();
        mPath->incNumberPointers();
    }

    ~AppendToPointsListUndoRedo() {
        mPath->decNumberPointers();
        mPoint->decNumberPointers();
    }

    void redo() {
        mPath->appendToPointsList(mPoint, false);
    }

    void undo() {
        mPath->removeFromPointsList(mPoint, false);
    }

private:
    PathPoint *mPoint;
    SinglePathAnimator *mPath;
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
        mNewNext = newNext;
        mOldNext = oldNext;
        mPoint = point;
        if(mNewNext != NULL) mNewNext->incNumberPointers();
        if(mOldNext != NULL) mOldNext->incNumberPointers();
        mPoint->incNumberPointers();
    }

    ~SetNextPointUndoRedo() {
        if(mNewNext != NULL) mNewNext->decNumberPointers();
        if(mOldNext != NULL) mOldNext->decNumberPointers();
        mPoint->decNumberPointers();
    }

    void redo(){
        mPoint->setNextPoint(mNewNext, false);
    }

    void undo() {
        mPoint->setNextPoint(mOldNext, false);
    }

private:
    PathPoint *mNewNext;
    PathPoint *mOldNext;
    PathPoint *mPoint;
};

class SetPreviousPointUndoRedo : public UndoRedo
{
public:
    SetPreviousPointUndoRedo(PathPoint *point,
                             PathPoint *oldPrevious,
                             PathPoint *newPrevious) :
        UndoRedo("SetPreviousPointUndoRedo") {
        mNewPrev = newPrevious;
        mOldPrev = oldPrevious;
        mPoint = point;
        if(mNewPrev != NULL) mNewPrev->incNumberPointers();
        if(mOldPrev != NULL) mOldPrev->incNumberPointers();
        mPoint->incNumberPointers();
    }

    ~SetPreviousPointUndoRedo() {
        if(mNewPrev != NULL) mNewPrev->decNumberPointers();
        if(mOldPrev != NULL) mOldPrev->decNumberPointers();
        mPoint->decNumberPointers();
    }

    void redo(){
        mPoint->setPreviousPoint(mNewPrev, false);
    }

    void undo() {
        mPoint->setPreviousPoint(mOldPrev, false);
    }

private:
    PathPoint *mNewPrev;
    PathPoint *mOldPrev;
    PathPoint *mPoint;
};

class AddPointToSeparatePathsUndoRedo : public UndoRedo
{
public:
    AddPointToSeparatePathsUndoRedo(PathAnimator *path,
                                    PathPoint *point) :
        UndoRedo("AddPointToSeparatePathsUndoRedo") {
        mPath = path;
        mPath->incNumberPointers();
        mPoint = point;
        mPoint->incNumberPointers();
    }

    ~AddPointToSeparatePathsUndoRedo() {
        mPath->decNumberPointers();
        mPoint->decNumberPointers();
    }

    void redo() {
        mPath->addPointToSeparatePaths(mPoint, false);
    }

    void undo() {
        mPath->removePointFromSeparatePaths(mPoint, false);
    }

private:
    PathAnimator *mPath;
    PathPoint *mPoint;
};

class RemovePointFromSeparatePathsUndoRedo :
        public AddPointToSeparatePathsUndoRedo
{
public:
    RemovePointFromSeparatePathsUndoRedo(PathAnimator *path,
                                         PathPoint *point) :
        AddPointToSeparatePathsUndoRedo(path, point) {

    }

    void redo() {
        AddPointToSeparatePathsUndoRedo::undo();
    }

    void undo() {
        AddPointToSeparatePathsUndoRedo::redo();
    }
};

class SetPathPointModeUndoRedo : public UndoRedo
{
public:
    SetPathPointModeUndoRedo(PathPoint *point,
                             CtrlsMode modeBefore,
                             CtrlsMode modeAfter) :
        UndoRedo("SetPathPointModeUndoRedo") {
        mPoint = point;
        mPoint->incNumberPointers();
        mBefore = modeBefore;
        mAfter = modeAfter;
    }

    ~SetPathPointModeUndoRedo() {
        mPoint->decNumberPointers();
    }

    void redo() {
        mPoint->setCtrlsMode(mAfter, false);
    }

    void undo() {
        mPoint->setCtrlsMode(mBefore, false);
    }

private:
    PathPoint *mPoint;
    CtrlsMode mBefore;
    CtrlsMode mAfter;
};

class SetCtrlPtEnabledUndoRedo : public UndoRedo
{
public:
    SetCtrlPtEnabledUndoRedo(bool enabled, bool isStartPt,
                             PathPoint* parentPoint) :
        UndoRedo("SetCtrlPtEnabledUndoRedo") {
        mParentPoint = parentPoint;
        mParentPoint->incNumberPointers();
        mEnabled = enabled;
        mIsStartPt = isStartPt;
    }

    ~SetCtrlPtEnabledUndoRedo() {
        mParentPoint->decNumberPointers();
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
                            int fromIndex,
                            int toIndex,
                            BoxesGroup *parentBox) :
        UndoRedo("MoveChildInListUndoRedo") {
        mParentBox = parentBox;
        mParentBox->incNumberPointers();
        mFromIndex = fromIndex;
        mToIndex = toIndex;
        mChild = child;
        mChild->incNumberPointers();
    }

    ~MoveChildInListUndoRedo() {
        mParentBox->decNumberPointers();
        mChild->decNumberPointers();
    }

    void redo() {
        mParentBox->moveChildInList(mChild, mFromIndex, mToIndex, false);
    }

    void undo() {
        mParentBox->moveChildInList(mChild, mToIndex, mFromIndex, false);
    }
private:
    BoxesGroup *mParentBox;
    BoundingBox *mChild;
    int mFromIndex;
    int mToIndex;
};

class MoveChildAnimatorInListUndoRedo : public UndoRedo
{
public:
    MoveChildAnimatorInListUndoRedo(QrealAnimator *child,
                            int fromIndex,
                            int toIndex,
                            ComplexAnimator *parentAnimator) :
        UndoRedo("MoveChildInListUndoRedo") {
        mParentAnimator = parentAnimator;
        mParentAnimator->incNumberPointers();
        mFromIndex = fromIndex;
        mToIndex = toIndex;
        mChild = child;
        mChild->incNumberPointers();
    }

    ~MoveChildAnimatorInListUndoRedo() {
        mParentAnimator->decNumberPointers();
        mChild->decNumberPointers();
    }

    void redo() {
        mParentAnimator->moveChildInList(mChild, mFromIndex, mToIndex, false);
    }

    void undo() {
        mParentAnimator->moveChildInList(mChild, mToIndex, mFromIndex, false);
    }
private:
    ComplexAnimator *mParentAnimator;
    QrealAnimator *mChild;
    int mFromIndex;
    int mToIndex;
};

class SetBoundingBoxZListIndexUnoRedo : public UndoRedo
{
public:
    SetBoundingBoxZListIndexUnoRedo(int indexBefore,
                                    int indexAfter,
                                    BoundingBox *box) :
        UndoRedo("SetBoundingBoxZListIndexUnoRedo") {
        mBox = box;
        mBox->incNumberPointers();
        mIndexAfter = indexAfter;
        mIndexBefore = indexBefore;
    }

    ~SetBoundingBoxZListIndexUnoRedo() {
        mBox->decNumberPointers();
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
    BoundingBox *mBox;
};

class AddChildToListUndoRedo : public UndoRedo
{
public:
    AddChildToListUndoRedo(BoxesGroup *parent,
                           int index,
                           BoundingBox *child) :
        UndoRedo("AddChildToListUndoRedo") {
        mParent = parent;
        mParent->incNumberPointers();
        mAddAtId = index;
        mChild = child;
        mChild->incNumberPointers();
    }

    ~AddChildToListUndoRedo() {
        mChild->decNumberPointers();
        mParent->decNumberPointers();
    }

    void redo() {
        mParent->addChildToListAt(mAddAtId, mChild, false);
    }

    void undo() {
        mParent->removeChildFromList(mAddAtId, false);
    }

private:
    BoxesGroup *mParent;
    int mAddAtId;
    BoundingBox *mChild;
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
        mChildBox = childBox;
        if(childBox != NULL) mChildBox->incNumberPointers();
        mOldParent = oldParent;
        if(oldParent != NULL) mOldParent->incNumberPointers();
        mNewParent = newParent;
        if(newParent != NULL) mNewParent->incNumberPointers();
    }

    ~SetBoxParentUndoRedo() {
        if(mChildBox != NULL) mChildBox->decNumberPointers();
        if(mOldParent != NULL) mOldParent->decNumberPointers();
        if(mNewParent != NULL) mNewParent->decNumberPointers();
    }

    void redo() {
        mChildBox->setParent(mNewParent, false);
    }

    void undo() {
        mChildBox->setParent(mOldParent, false);
    }

private:
    BoundingBox *mChildBox;
    BoxesGroup *mOldParent;
    BoxesGroup *mNewParent;
};

class SetPivotRelPosUndoRedo : public UndoRedo
{
public:
    SetPivotRelPosUndoRedo(BoundingBox *target,
                           QPointF prevRelPos, QPointF newRelPos,
                           bool prevPivotChanged, bool newPivotChanged) :
        UndoRedo("SetPivotRelPosUndoRedo") {
        mTarget = target;
        mTarget->incNumberPointers();
        mPrevRelPos = prevRelPos;
        mNewRelPos = newRelPos;
        mPrevPivotChanged = prevPivotChanged;
        mNewPivotChanged = newPivotChanged;
    }

    ~SetPivotRelPosUndoRedo() {
        mTarget->decNumberPointers();
    }

    void redo() {
        mTarget->setPivotRelPos(mNewRelPos, false, mNewPivotChanged);
    }

    void undo() {
        mTarget->setPivotRelPos(mPrevRelPos, false, mPrevPivotChanged);
    }

private:
    BoundingBox *mTarget;
    QPointF mPrevRelPos;
    QPointF mNewRelPos;
    bool mPrevPivotChanged;
    bool mNewPivotChanged;
};

class  SetBoxVisibleUndoRedo : public UndoRedo
{
public:
     SetBoxVisibleUndoRedo(BoundingBox *target,
                           bool visibleBefore, bool visibleAfter) :
         UndoRedo("SetBoxVisibleUndoRedo") {
         mTarget = target;
         mTarget->incNumberPointers();
         mVisibleAfter = visibleAfter;
         mVisibleBefore = visibleBefore;
     }

     ~SetBoxVisibleUndoRedo() {
         mTarget->decNumberPointers();
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
     BoundingBox *mTarget;
};

class ChangeQrealAnimatorValue : public UndoRedo
{
public:
    ChangeQrealAnimatorValue(qreal oldValue, qreal newValue,
                             QrealAnimator *animator) :
        UndoRedo("ChangeQrealAnimatorValue") {
        mOldValue = oldValue;
        mNewValue = newValue;
        mAnimator = animator;
        mAnimator->incNumberPointers();
    }

    ~ChangeQrealAnimatorValue() {
        mAnimator->decNumberPointers();
    }

    void redo() {
        mAnimator->setCurrentValue(mNewValue);
    }

    void undo() {
        mAnimator->setCurrentValue(mOldValue);
    }

private:
    qreal mOldValue;
    qreal mNewValue;
    QrealAnimator *mAnimator;
};

class ChangeQrealKeyValueUndoRedo : public UndoRedo
{
public:
    ChangeQrealKeyValueUndoRedo(qreal oldValue, qreal newValue, QrealKey *key) :
        UndoRedo("ChangeQrealKeyValueUndoRedo") {
        mOldValue = oldValue;
        mNewValue = newValue;
        mTargetKey = key;
        key->incNumberPointers();
    }

    ~ChangeQrealKeyValueUndoRedo() {
        mTargetKey->decNumberPointers();
    }

    void redo() {
        mTargetKey->getParentAnimator()->saveValueToKey(mTargetKey, mNewValue, false);
    }

    void undo() {
        mTargetKey->getParentAnimator()->saveValueToKey(mTargetKey, mOldValue, false);
    }

private:
    qreal mOldValue;
    qreal mNewValue;
    QrealKey *mTargetKey;
};

class ChangeQrealKeyFrameUndoRedo : public UndoRedo
{
public:
    ChangeQrealKeyFrameUndoRedo(int oldFrame, int newFrame, QrealKey *key) :
        UndoRedo("ChangeQrealKeyFrameUndoRedo") {
        mOldFrame = oldFrame;
        mNewFrame = newFrame;
        mTargetKey = key;
        key->incNumberPointers();
    }

    ~ChangeQrealKeyFrameUndoRedo() {
        mTargetKey->decNumberPointers();
    }

    void redo() {
        mTargetKey->getParentAnimator()->moveKeyToFrame(mTargetKey, mNewFrame);
    }

    void undo() {
        mTargetKey->getParentAnimator()->moveKeyToFrame(mTargetKey, mOldFrame);
    }

private:
    int mOldFrame;
    int mNewFrame;
    QrealKey *mTargetKey;
};

class QrealAnimatorRecordingSetUndoRedo : public UndoRedo
{
public:
    QrealAnimatorRecordingSetUndoRedo(bool recordingOld,
                                      bool recordingNew,
                                      QrealAnimator *animator) :
    UndoRedo("QrealAnimatorRecordingSetUndoRedo") {
        mRecordingOld = recordingOld;
        mRecordingNew = recordingNew;
        mAnimator = animator;
        mAnimator->incNumberPointers();
    }

    ~QrealAnimatorRecordingSetUndoRedo() {
        mAnimator->decNumberPointers();
    }

    void undo() {
        mAnimator->setRecordingWithoutChangingKeys(mRecordingOld, false);
    }

    void redo() {
        mAnimator->setRecordingWithoutChangingKeys(mRecordingNew, false);
    }

private:
    bool mRecordingOld;
    bool mRecordingNew;
    QrealAnimator *mAnimator;
};

class AddQrealKeyToAnimatorUndoRedo : public UndoRedo
{
public:
    AddQrealKeyToAnimatorUndoRedo(QrealKey *key, QrealAnimator *animator) :
        UndoRedo("AddQrealKeyToAnimatorUndoRedo") {
        mKey = key;
        mAnimator = animator;
        mAnimator->incNumberPointers();
        mKey->incNumberPointers();
    }

    ~AddQrealKeyToAnimatorUndoRedo() {
        mAnimator->decNumberPointers();
        mKey->decNumberPointers();
    }

    void redo() {
        mAnimator->appendKey(mKey, false);
    }

    void undo() {
        mAnimator->removeKey(mKey, false);
    }

private:
    QrealKey *mKey;
    QrealAnimator *mAnimator;
};

class RemoveQrealKeyFromAnimatorUndoRedo : public AddQrealKeyToAnimatorUndoRedo
{
public:
    RemoveQrealKeyFromAnimatorUndoRedo(QrealKey *key, QrealAnimator *animator) :
        AddQrealKeyToAnimatorUndoRedo(key, animator) {

    }

    void redo() {
        AddQrealKeyToAnimatorUndoRedo::undo();
    }

    void undo() {
        AddQrealKeyToAnimatorUndoRedo::redo();
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
        mTarget = target;
        mTarget->incNumberPointers();
    }

    ~PaintTypeChangeUndoRedo() {
        mTarget->decNumberPointers();
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
    PaintSettings *mTarget;
};

class GradientChangeUndoRedo : public UndoRedo {
public:
    GradientChangeUndoRedo(Gradient *oldGradient,
                           Gradient *newGradient,
                           PaintSettings *target) :
        UndoRedo("GradientChangeUndoRedo") {
        mTarget = target;
        mTarget->incNumberPointers();
        mOldGradient = oldGradient;
        mNewGradient = newGradient;
    }

    ~GradientChangeUndoRedo() {
        mTarget->decNumberPointers();
    }

    void redo() {
        mTarget->setGradient(mNewGradient, false);
    }

    void undo() {
        mTarget->setGradient(mOldGradient, false);
    }

private:
    Gradient *mOldGradient;
    Gradient *mNewGradient;
    PaintSettings *mTarget;
};


class GradientColorAddedToListUndoRedo : public UndoRedo {
public:
    GradientColorAddedToListUndoRedo(Gradient *target,
                                     ColorAnimator *color) :
        UndoRedo("GradientColorAddedToListUndoRedo") {
        mGradient = target;
        mColor = color;
        mColor->incNumberPointers();
        mGradient->incNumberPointers();
    }

    ~GradientColorAddedToListUndoRedo() {
        mColor->decNumberPointers();
        mGradient->decNumberPointers();
    }

    void undo() {
        mGradient->removeColor(mColor, false);
    }

    void redo() {
        mGradient->addColorToList(mColor, false);
    }

private:
    ColorAnimator *mColor;
    Gradient *mGradient;
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
                               const int id1,
                               const int id2) :
        UndoRedo("GradientSwapColorsUndoRedo") {
        mGradient = target;
        mGradient->incNumberPointers();
        mId1 = id1;
        mId2 = id2;
    }

    ~GradientSwapColorsUndoRedo() {
        mGradient->decNumberPointers();
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
    Gradient *mGradient;
};

class AddSinglePathAnimatorUndoRedo : public UndoRedo {
public:
    AddSinglePathAnimatorUndoRedo(PathAnimator *target,
                                  SinglePathAnimator *path) :
        UndoRedo("AddSinglePathAnimatorUndoRedo") {
        mTarget = target;
        mPath = path;
        target->incNumberPointers();
        path->incNumberPointers();
    }

    ~AddSinglePathAnimatorUndoRedo() {
        mTarget->decNumberPointers();
        mPath->decNumberPointers();
    }

    void undo() {
        mTarget->removeSinglePathAnimator(mPath, false);
    }

    void redo() {
        mTarget->addSinglePathAnimator(mPath, false);
    }
private:
    PathAnimator *mTarget;
    SinglePathAnimator *mPath;
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
        mTarget = target;
        mOldPoint = oldPoint;
        mNewPoint = newPoint;
        if(oldPoint != NULL) {
            oldPoint->incNumberPointers();
        }
        if(newPoint != NULL) {
            newPoint->incNumberPointers();
        }
        target->incNumberPointers();
    }

    ~ChangeSinglePathFirstPoint() {
        if(mNewPoint != NULL) {
            mNewPoint->decNumberPointers();
        }
        if(mOldPoint != NULL) {
            mOldPoint->decNumberPointers();
        }
        mTarget->decNumberPointers();
    }

    void undo() {
        mTarget->replaceSeparatePathPoint(mOldPoint, false);
    }

    void redo() {
        mTarget->replaceSeparatePathPoint(mNewPoint, false);
    }

private:
    SinglePathAnimator *mTarget;
    PathPoint *mOldPoint;
    PathPoint *mNewPoint;
};

class ReversePointsDirectionUndoRedo :
        public UndoRedo {
public:
    ReversePointsDirectionUndoRedo(PathPoint *target) :
        UndoRedo("ReversePointsDirectionUndoRedo") {
        mTarget = target;
        mTarget->incNumberPointers();
    }

    ~ReversePointsDirectionUndoRedo() {
        mTarget->decNumberPointers();
    }

    void undo() {
        mTarget->reversePointsDirectionReverse();
    }

    void redo() {
        mTarget->reversePointsDirection();
    }
private:
    PathPoint *mTarget;
};

#endif // UNDOREDO_H
