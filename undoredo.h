#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include "canvas.h"
#include "vectorpath.h"

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
    UndoRedoSet() : UndoRedo() {}
    void undo() {
        foreach(UndoRedo undoRedo, mSet) {
            undoRedo.undo();
        }
    }

    void redo() {
        foreach(UndoRedo undoRedo, mSet) {
            undoRedo.redo();
        }
    }

    void addUndoRedo(UndoRedo undoRedo) {
        mSet << undoRedo;
    }

private:
    QList<UndoRedo> mSet;
};

class UndoRedoStack {
public:
    UndoRedoStack() {}
    void startNewSet() {
        mCurrentSet = UndoRedoSet();
        mAddToSet = true;
    }

    void finishSet() {
        mAddToSet = false;
        addSet();
    }

    void addSet() {
        addUndoRedo((UndoRedo) mCurrentSet);
    }

    void addToSet(UndoRedo undoRedo) {
        mCurrentSet.addUndoRedo(undoRedo);
    }

    void addUndoRedo(UndoRedo undoRedo) {
        if(mAddToSet) {
            addToSet(undoRedo);
        } else {
            mUndoStack << undoRedo;
        }
    }

    void redo() {
        if(mRedoStack.isEmpty()) {
            return;
        }
        UndoRedo toRedo = mRedoStack.takeLast();
        toRedo.redo();
        mUndoStack << toRedo;
    }
    void undo() {
        if(mUndoStack.isEmpty()) {
            return;
        }
        UndoRedo toUndo = mUndoStack.takeLast();
        toUndo.undo();
        mRedoStack << toUndo;
    }

private:
    bool mAddToSet = false;
    UndoRedoSet mCurrentSet;
    QList<UndoRedo> mUndoStack;
    QList<UndoRedo> mRedoStack;
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

class DeletePathUndoRedo : public UndoRedo
{
public:
    DeletePathUndoRedo(VectorPath *deletedPath) : UndoRedo() {
        mDeletedPath = deletedPath;
    }

    void redo() {
        mDeletedPath->getCanvas()->removePath(mDeletedPath);
    }

    void undo() {
        mDeletedPath->getCanvas()->addPath(mDeletedPath);
    }

private:
    VectorPath *mDeletedPath;
};

class MovePathPointUndoRedo : public UndoRedo
{
public:
    MovePathPointUndoRedo(PathPoint *movedPoint,
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
    PathPoint *mMovedPoint;
    QPointF mAbsPosBefore;
    QPointF mAbsPosAfter;
};

class CreatePointUndoRedo : public UndoRedo
{
public:
    CreatePointUndoRedo(PathPoint *createdPoint) : UndoRedo(){
        mCreatedPoint = createdPoint;
    }

    virtual void redo() {
        PathPoint *addToPoint = NULL;
        if(mCreatedPoint->hasPreviousPoint()) {
            addToPoint = mCreatedPoint->getPreviousPoint();
        } else if(mCreatedPoint->hasNextPoint()) {
            addToPoint = mCreatedPoint->getNextPoint();
        }
        mCreatedPoint->getParentPath()->addPoint(mCreatedPoint, addToPoint);
    }

    virtual void undo() {
        mCreatedPoint->getParentPath()->removePoint(mCreatedPoint);
    }

private:
    PathPoint *mCreatedPoint;
};

class DeletePointUndoRedo : public CreatePointUndoRedo
{
public:
    DeletePointUndoRedo(PathPoint *deletedPoint) : CreatePointUndoRedo(deletedPoint) {

    }

    void redo() {
        CreatePointUndoRedo::redo();
    }

    void undo() {
        CreatePointUndoRedo::undo();
    }
};

#endif // UNDOREDO_H
