#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include "smartPointers/sharedpointerdefs.h"

class UndoRedo : public StdSelfRef {
public:
    UndoRedo(const QString &name);
    virtual ~UndoRedo();
    virtual void undo();
    virtual void redo();
    void printName();
    void printUndoName();
    void printRedoName();

    void setFrame(const int& frame) {
        mFrame = frame;
    }
    int getFrame();
private:
    int mFrame;
    QString mName;
};

class UndoRedoSet : public UndoRedo {
public:
    UndoRedoSet(const int& absFrame);
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

    void setFrame(const int& frame) {
        mCurrentAbsFrame = frame;
    }
private:
    int mCurrentAbsFrame;
    std::function<bool(int)> mChangeFrameFunc;
    bool addSet();
    void addToSet(const stdsptr<UndoRedo> &undoRedo);

    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = INT_MAX;
    int mNumberOfSets = 0;

    stdsptr<UndoRedoSet> mCurrentSet;
    QList<stdsptr<UndoRedo>> mUndoStack;
    QList<stdsptr<UndoRedo>> mRedoStack;
};
#endif // UNDOREDO_H
