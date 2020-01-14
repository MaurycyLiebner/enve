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

#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include "smartPointers/ememory.h"
#include "framerange.h"

class UndoRedo_priv;
class UndoRedoSet;

class UndoRedoStack : public StdSelfRef {
public:
    UndoRedoStack(const std::function<bool(int)>& changeFrameFunc);

    void startNewSet();
    bool finishSet();

    void addUndoRedo(const std::function<void()> &undo,
                     const std::function<void()> &redo);

    bool redo();
    bool undo();
    void emptySomeOfUndo();

    void blockUndoRedo();
    void unblockUndoRedo();
    bool undoRedoBlocked();

    void setFrame(const int frame)
    { mCurrentAbsFrame = frame; }
private:
    bool addSet();
    void addToSet(const stdsptr<UndoRedo_priv> &undoRedo);
    void addUndoRedo(const stdsptr<UndoRedo_priv>& undoRedo);

    int mCurrentAbsFrame;
    const std::function<bool(int)> mChangeFrameFunc;

    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = FrameRange::EMAX;
    int mNumberOfSets = 0;

    stdsptr<UndoRedoSet> mCurrentSet;
    QList<stdsptr<UndoRedo_priv>> mUndoStack;
    QList<stdsptr<UndoRedo_priv>> mRedoStack;
};

#endif // UNDOREDO_H
