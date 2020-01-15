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

#include "undoredo.h"
#include "exceptions.h"

class UndoRedo_priv {
public:
    UndoRedo_priv(const int frame,
                  const std::function<void()>& undo,
                  const std::function<void()>& redo) :
        fFrame(frame), fUndo(undo), fRedo(redo) {}

    const int fFrame;
    const std::function<void()> fUndo;
    const std::function<void()> fRedo;
};

class UndoRedoSet : public UndoRedo_priv {
public:
    UndoRedoSet(const int frame) :
        UndoRedo_priv(frame,
                 [this]() { undo(); },
                 [this]() { redo(); }) {}

    void addUndoRedo(const stdsptr<UndoRedo_priv>& undoRedo)
    { mSet << undoRedo; }
    bool isEmpty()
    { return mSet.isEmpty(); }
private:
    void undo();
    void redo();

    QList<stdsptr<UndoRedo_priv>> mSet;
};

void UndoRedoSet::undo() {
    for(int i = mSet.length() - 1; i >= 0; i--)
        mSet.at(i)->fUndo();
}

void UndoRedoSet::redo() {
    for(const auto& undoRedo : mSet)
        undoRedo->fRedo();
}

UndoRedoStack::UndoRedoStack(const std::function<bool(int)> &changeFrameFunc) :
    mChangeFrameFunc(changeFrameFunc) {
    startNewSet();
}

void UndoRedoStack::startNewSet() {
    mNumberOfSets++;
}

bool UndoRedoStack::finishSet() {
    mNumberOfSets--;
    if(mNumberOfSets == 0) return addSet();
    return false;
}

bool UndoRedoStack::addSet() {
    const bool add = mCurrentSet && !mCurrentSet->isEmpty();
    if(add) addUndoRedo(mCurrentSet);
    mCurrentSet = nullptr;
    return add;
}

void UndoRedoStack::addToSet(const stdsptr<UndoRedo_priv>& undoRedo) {
    if(!mCurrentSet) {
        mCurrentSet = std::make_shared<UndoRedoSet>(mCurrentAbsFrame);
    }
    mCurrentSet->addUndoRedo(undoRedo);
}

void UndoRedoStack::emptySomeOfUndo() {
    if(mUndoStack.length() > 150) {
        //            for(int i = 0; i < 50; i++) {
        mUndoStack.removeFirst();
        //            }
    }
}

void UndoRedoStack::addUndoRedo(const std::function<void()>& undoFunc,
                                const std::function<void()>& redoFunc) {
    if(mUndoRedoBlocked) return;
    if(!undoFunc) RuntimeThrow("Missing undo function.");
    if(!redoFunc) RuntimeThrow("Missing redo function.");
    const auto undoRedo = std::make_shared<UndoRedo_priv>(mCurrentAbsFrame,
                                                          undoFunc, redoFunc);
    addUndoRedo(undoRedo);
}

void UndoRedoStack::addUndoRedo(const stdsptr<UndoRedo_priv>& undoRedo) {
    if(mUndoRedoBlocked) return;
    if(mNumberOfSets != 0) {
        addToSet(undoRedo);
    } else {
        mRedoStack.clear();
        emptySomeOfUndo();
        mUndoStack << undoRedo;
    }
}

bool UndoRedoStack::redo() {
    if(mRedoStack.isEmpty()) return false;
    stdsptr<UndoRedo_priv> toRedo = mRedoStack.last();
    if(mChangeFrameFunc(toRedo->fFrame)) return true;
    mRedoStack.removeLast();
    {
        const auto block = blockUndoRedo();
        toRedo->fRedo();
    }
    mUndoStack << toRedo;
    return true;
}

bool UndoRedoStack::undo() {
    if(mUndoStack.isEmpty()) return false;
    stdsptr<UndoRedo_priv> toUndo = mUndoStack.last();
    if(mChangeFrameFunc(toUndo->fFrame)) return true;
    mUndoStack.removeLast();
    {
        const auto block = blockUndoRedo();
        toUndo->fUndo();
    }
    mRedoStack << toUndo;
    return true;
}

UndoRedoStack::StackBlock UndoRedoStack::blockUndoRedo() {
    return StackBlock(*this);
}
