// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

UndoRedoStack::UndoRedoStack(const std::function<bool(int)> &changeFrameFunc) :
    mChangeFrameFunc(changeFrameFunc) {
    startNewSet();
}

void UndoRedoStack::startNewSet() {
    mNumberOfSets++;
}

UndoRedoStack::~UndoRedoStack() {
    clearUndoStack();
    clearRedoStack();
}

bool UndoRedoStack::finishSet() {
    mNumberOfSets--;
    if(mNumberOfSets == 0) {
        return addSet();
    }
    return false; // !!! when is this needed??
}

bool UndoRedoStack::addSet() {
    if((mCurrentSet == nullptr) ? true : mCurrentSet->isEmpty()) {
        mCurrentSet = nullptr;
        return false;
    }
    addUndoRedo(mCurrentSet);
    mCurrentSet = nullptr;
    return true;
}

void UndoRedoStack::addToSet(const stdsptr<UndoRedo>& undoRedo) {
    if(!mCurrentSet) {
        mCurrentSet = enve::make_shared<UndoRedoSet>(mCurrentAbsFrame);
    }
    mCurrentSet->addUndoRedo(undoRedo);
}

void UndoRedoStack::clearRedoStack() {
    mRedoStack.clear();
}

void UndoRedoStack::clearUndoStack() {
    mUndoStack.clear();
}

void UndoRedoStack::emptySomeOfUndo() {
    if(mUndoStack.length() > 150) {
        //            for(int i = 0; i < 50; i++) {
        mUndoStack.removeFirst();
        //            }
    }
}

void UndoRedoStack::clearAll() {
    clearRedoStack();
    clearUndoStack();
}

void UndoRedoStack::addUndoRedo(const stdsptr<UndoRedo> &undoRedo) {
    if(!undoRedo) return;
    undoRedo->setFrame(mCurrentAbsFrame);
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
//            bool noSet = mCurrentSet == nullptr;

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
    stdsptr<UndoRedo> toRedo = mRedoStack.last();
    if(mChangeFrameFunc(toRedo->getFrame())) return;
    mRedoStack.removeLast();
    toRedo->printRedoName();
    toRedo->redo();
    mUndoStack << toRedo;
}

void UndoRedoStack::undo() {
    if(mUndoStack.isEmpty()) {
        return;
    }
    stdsptr<UndoRedo> toUndo = mUndoStack.last();
    if(mChangeFrameFunc(toUndo->getFrame())) return;
    mUndoStack.removeLast();
    toUndo->printUndoName();
    toUndo->undo();
    mRedoStack << toUndo;
}

void UndoRedoStack::blockUndoRedo() {
    mUndoRedoBlocked = true;
}

void UndoRedoStack::unblockUndoRedo() {
    mUndoRedoBlocked = false;
}

bool UndoRedoStack::undoRedoBlocked() {
    return mUndoRedoBlocked;
}

UndoRedo::UndoRedo(const QString &name) {
    mName = name;
    printName();
}

UndoRedo::~UndoRedo() {
    qDebug() << "DELETE " << mName;
}

void UndoRedo::undo() {}

void UndoRedo::redo() {}

void UndoRedo::printName() { qDebug() << mName; }

void UndoRedo::printUndoName() { qDebug() << "UNDO " << mName; }

void UndoRedo::printRedoName() { qDebug() << "REDO " << mName; }

int UndoRedo::getFrame() { return mFrame; }

UndoRedoSet::UndoRedoSet(const int absFrame) :
    UndoRedo("UndoRedoSet") {
    setFrame(absFrame);
}

UndoRedoSet::~UndoRedoSet() {}

void UndoRedoSet::undo() {
    for(int i = mSet.length() - 1; i >= 0; i--) {
        mSet.at(i)->undo();
    }
}

void UndoRedoSet::redo() {
    for(const auto& undoRedo : mSet) {
        undoRedo->redo();
    }
}

void UndoRedoSet::addUndoRedo(const stdsptr<UndoRedo> &undoRedo) {
    mSet << undoRedo;
}

bool UndoRedoSet::isEmpty() {
    return mSet.isEmpty();
}
