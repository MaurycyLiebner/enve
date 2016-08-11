#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include "canvas.h"
#include "vectorpath.h"
#include "movablepoint.h"
#include <QDebug>

class UndoRedo
{
public:
    UndoRedo() {}
    virtual ~UndoRedo() {}
    virtual void undo() {}
    virtual void redo() {}
};

class UndoRedoSet : public UndoRedo
{
public:
    UndoRedoSet() : UndoRedo() {
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

private:
    QList<UndoRedo*> mSet;
};

class UndoRedoStack {
public:
    UndoRedoStack() {}
    void startNewSet() {
        if(mNumberOfSets == 0) {
            mCurrentSet = new UndoRedoSet();
        }
        mNumberOfSets++;
    }

    void finishSet() {
        mNumberOfSets--;
        if(mNumberOfSets == 0) {
            addSet();
            mCurrentSet = NULL;
        }
    }

    void addSet() {
        addUndoRedo(mCurrentSet);
    }

    void addToSet(UndoRedo *undoRedo) {
        mCurrentSet->addUndoRedo(undoRedo);
    }

    void clearRedoStack() {
        foreach (UndoRedo *redoStackItem, mRedoStack) {
            delete redoStackItem;
        }
        mRedoStack.clear();
    }

    void addUndoRedo(UndoRedo *undoRedo) {
        if(mNumberOfSets != 0) {
            addToSet(undoRedo);
        } else {
            clearRedoStack();
            mUndoStack << undoRedo;
        }
    }

    void redo() {
        if(mRedoStack.isEmpty()) {
            return;
        }
        UndoRedo *toRedo = mRedoStack.takeLast();
        toRedo->redo();
        mUndoStack << toRedo;
    }
    void undo() {
        if(mUndoStack.isEmpty()) {
            return;
        }
        UndoRedo *toUndo = mUndoStack.takeLast();
        toUndo->undo();
        mRedoStack << toUndo;
    }

private:
    int mNumberOfSets = 0;
    UndoRedoSet *mCurrentSet;
    QList<UndoRedo*> mUndoStack;
    QList<UndoRedo*> mRedoStack;
};

class TransformChildParentUndoRedo : public UndoRedo
{
public:
    TransformChildParentUndoRedo(BoundingBox *transformedPath,
                     QMatrix transformBefore,
                     QMatrix transformAfter) : UndoRedo() {
        mTransformedPath = transformedPath;
        mTransformBefore = transformBefore;
        mTransformAfter = transformAfter;
    }

    void redo() {
        mTransformedPath->setTransformation(mTransformAfter);
    }

    void undo() {
        mTransformedPath->setTransformation(mTransformBefore);
    }

private:
    BoundingBox *mTransformedPath;
    QMatrix mTransformBefore;
    QMatrix mTransformAfter;
};

class MoveMovablePointUndoRedo : public UndoRedo
{
public:
    MoveMovablePointUndoRedo(MovablePoint *movedPoint,
                         QPointF relPosBefore,
                         QPointF relPosAfter) : UndoRedo() {
        mMovedPoint = movedPoint;
        mRelPosAfter = relPosAfter;
        mRelPosBefore = relPosBefore;
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
    AppendToPointsListUndoRedo(PathPoint *pointToAdd, VectorPath *path) : UndoRedo() {
        mPoint = pointToAdd;
        mPath = path;
    }

    void redo() {
        mPath->appendToPointsList(mPoint, false);
    }

    void undo() {
        mPath->removeFromPointsList(mPoint, false);
    }

private:
    PathPoint *mPoint;
    VectorPath *mPath;
};

class RemoveFromPointsListUndoRedo : public AppendToPointsListUndoRedo
{
public:
    RemoveFromPointsListUndoRedo(PathPoint *pointToRemove, VectorPath *path) : AppendToPointsListUndoRedo(pointToRemove, path) {
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
    SetNextPointUndoRedo(PathPoint *point, PathPoint *oldNext, PathPoint *newNext) : UndoRedo() {
        mNewNext = newNext;
        mOldNext = oldNext;
        mPoint = point;
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
    SetPreviousPointUndoRedo(PathPoint *point, PathPoint *oldPrevious, PathPoint *newPrevious) : UndoRedo() {
        mNewPrev = newPrevious;
        mOldPrev = oldPrevious;
        mPoint = point;
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
    AddPointToSeparatePathsUndoRedo(VectorPath *path, PathPoint *point) : UndoRedo() {
        mPath = path;
        mPoint = point;
    }

    void redo() {
        mPath->addPointToSeparatePaths(mPoint, false);
    }

    void undo() {
        mPath->removePointFromSeparatePaths(mPoint, false);
    }

private:
    VectorPath *mPath;
    PathPoint *mPoint;
};

class RemovePointFromSeparatePathsUndoRedo : public AddPointToSeparatePathsUndoRedo
{
public:
    RemovePointFromSeparatePathsUndoRedo(VectorPath *path, PathPoint *point) : AddPointToSeparatePathsUndoRedo(path, point) {

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
    SetPathPointModeUndoRedo(PathPoint *point, CtrlsMode modeBefore, CtrlsMode modeAfter) : UndoRedo() {
        mPoint = point;
        mBefore = modeBefore;
        mAfter = modeAfter;
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
    SetCtrlPtEnabledUndoRedo(bool enabled, bool isStartPt, PathPoint* parentPoint) : UndoRedo() {
        mParentPoint = parentPoint;
        mEnabled = enabled;
        mIsStartPt = isStartPt;
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
    MoveChildInListUndoRedo(int fromIndex,
                            int toIndex,
                            BoundingBox *parentBox) : UndoRedo() {
        mParentBox = parentBox;
        mFromIndex = fromIndex;
        mToIndex = toIndex;
    }

    void redo() {
        mParentBox->moveChildInList(mFromIndex, mToIndex, false);
    }

    void undo() {
        mParentBox->moveChildInList(mToIndex, mFromIndex, false);
    }
private:
    BoundingBox *mParentBox;
    int mFromIndex;
    int mToIndex;
};

class SetBoundingBoxZListIndexUnoRedo : public UndoRedo
{
public:
    SetBoundingBoxZListIndexUnoRedo(int indexBefore,
                                    int indexAfter,
                                    BoundingBox *box) : UndoRedo() {
        mBox = box;
        mIndexAfter = indexAfter;
        mIndexBefore = indexBefore;
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
    AddChildToListUndoRedo(BoundingBox *parent,
                           int index,
                           BoundingBox *child) : UndoRedo() {
        mParent = parent;
        mAddAtId = index;
        mChild = child;
    }

    void redo() {
        mParent->addChildToListAt(mAddAtId, mChild, false);
    }

    void undo() {
        mParent->removeChildFromList(mAddAtId, false);
    }

private:
    BoundingBox *mParent;
    int mAddAtId;
    BoundingBox *mChild;
};

class RemoveChildFromListUndoRedo : public AddChildToListUndoRedo
{
public:
    RemoveChildFromListUndoRedo(BoundingBox *parent,
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

#endif // UNDOREDO_H
