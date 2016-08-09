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
    TransformChildParentUndoRedo(ChildParent *transformedPath,
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
    ChildParent *mTransformedPath;
    QMatrix mTransformBefore;
    QMatrix mTransformAfter;
};

class AddPathUndoRedo : public UndoRedo
{
public:
    AddPathUndoRedo(VectorPath *createdPath) : UndoRedo() {
        mCreatedPath = createdPath;
    }

    void redo() {
        mCreatedPath->getCanvas()->addPath(mCreatedPath, false);
    }

    void undo() {
        mCreatedPath->getCanvas()->removePath(mCreatedPath, false);
    }

private:
    VectorPath *mCreatedPath;
};

class RemovePathUndoRedo : public AddPathUndoRedo
{
public:
    RemovePathUndoRedo(VectorPath *removedPath) : AddPathUndoRedo(removedPath) {
    }

    void redo() {
        AddPathUndoRedo::undo();
    }

    void undo() {
        AddPathUndoRedo::redo();
    }

private:
    VectorPath *mDeletedPath;
};

class MoveMovablePointUndoRedo : public UndoRedo
{
public:
    MoveMovablePointUndoRedo(MovablePoint *movedPoint,
                         QPointF absPosBefore,
                         QPointF absPosAfter) : UndoRedo() {
        mMovedPoint = movedPoint;
        mAbsPosAfter = absPosAfter;
        mAbsPosBefore = absPosBefore;
    }

    void redo() {
        mMovedPoint->setAbsolutePos(mAbsPosAfter);
    }

    void undo() {
        mMovedPoint->setAbsolutePos(mAbsPosBefore);
    }

private:
    MovablePoint *mMovedPoint;
    QPointF mAbsPosBefore;
    QPointF mAbsPosAfter;
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

#endif // UNDOREDO_H
