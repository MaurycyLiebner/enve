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

class UndoRedo : public StdSelfRef {
public:
    UndoRedo(const QString &name);
    virtual ~UndoRedo();
    virtual void undo();
    virtual void redo();
    void printName();
    void printUndoName();
    void printRedoName();

    void setFrame(const int frame) {
        mFrame = frame;
    }
    int getFrame();
private:
    int mFrame;
    QString mName;
};

class UndoRedoSet : public UndoRedo {
public:
    UndoRedoSet(const int absFrame);
    ~UndoRedoSet();

    void undo();
    void redo();

    void addUndoRedo(const stdsptr<UndoRedo>& undoRedo);
    bool isEmpty();
private:
    QList<stdsptr<UndoRedo>> mSet;
};

class UndoRedoStack : public StdSelfRef {
public:
    UndoRedoStack(const std::function<bool(int)>& changeFrameFunc);
    ~UndoRedoStack();

    void startNewSet();
    bool finishSet();

    void clearRedoStack();
    void clearUndoStack();
    void clearAll();

    void addUndoRedo(const stdsptr<UndoRedo>& undoRedo);

    void redo();
    void undo();
    void emptySomeOfUndo();

    void blockUndoRedo();
    void unblockUndoRedo();
    bool undoRedoBlocked();

    void setFrame(const int frame) {
        mCurrentAbsFrame = frame;
    }
private:
    int mCurrentAbsFrame;
    std::function<bool(int)> mChangeFrameFunc;
    bool addSet();
    void addToSet(const stdsptr<UndoRedo> &undoRedo);

    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = FrameRange::EMAX;
    int mNumberOfSets = 0;

    stdsptr<UndoRedoSet> mCurrentSet;
    QList<stdsptr<UndoRedo>> mUndoStack;
    QList<stdsptr<UndoRedo>> mRedoStack;
};
#endif // UNDOREDO_H
