#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include <QDebug>
#include <memory>
#include <QFont>
#include "skiaincludes.h"

#include "Animators/PathAnimators/vectorpathanimator.h"
#include "smartPointers/sharedpointerdefs.h"
class MainWindow;
class Tile;

enum CtrlsMode : short;
enum PaintType : short;

class UndoRedo {
public:
    UndoRedo(const QString &name/* = ""*/);
    virtual ~UndoRedo() {
        qDebug() << "DELETE " << mName;
    }
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

class UndoRedoSet : public UndoRedo {
public:
    UndoRedoSet() : UndoRedo("UndoRedoSet") {
    }

    ~UndoRedoSet() {
        Q_FOREACH(UndoRedo *undoRedo, mSet) {
            delete undoRedo;
        }
    }

    void undo() {
        for(int i = mSet.length() - 1; i >= 0; i--) {
            mSet.at(i)->undo();
        }
    }

    void redo() {
        Q_FOREACH(UndoRedo* undoRedo, mSet) {
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
    UndoRedoStack(MainWindow *mainWindow) {
        mMainWindow = mainWindow;
        startNewSet();
    }

    void startNewSet() {
        mNumberOfSets++;
    }

    ~UndoRedoStack() {
        clearUndoStack();
        clearRedoStack();
    }

    void finishSet() {
        mNumberOfSets--;
        if(mNumberOfSets == 0) {
            addSet();
        }
    }

    void addSet() {
        if((mCurrentSet == nullptr) ? true : mCurrentSet->isEmpty()) {
            mCurrentSet = nullptr;
            return;
        }
        addUndoRedo(mCurrentSet);
        mCurrentSet = nullptr;
    }

    void addToSet(UndoRedo *undoRedo) {
        if(mCurrentSet == nullptr) {
            mCurrentSet = new UndoRedoSet();
        }
        mCurrentSet->addUndoRedo(undoRedo);
    }

    void clearRedoStack() {
        Q_FOREACH (UndoRedo *redoStackItem, mRedoStack) {
            delete redoStackItem;
        }
        mRedoStack.clear();
    }

    void clearUndoStack() {
        Q_FOREACH (UndoRedo *undoStackItem, mUndoStack) {
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

    void blockUndoRedo() {
        mUndoRedoBlocked = true;
    }

    void unblockUndoRedo() {
        mUndoRedoBlocked = false;
    }

    bool undoRedoBlocked() {
        return mUndoRedoBlocked;
    }
private:
    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = INT_MAX;
    MainWindow *mMainWindow;
    int mNumberOfSets = 0;
    UndoRedoSet *mCurrentSet = nullptr;
    QList<UndoRedo*> mUndoStack;
    QList<UndoRedo*> mRedoStack;
};
#endif // UNDOREDO_H
