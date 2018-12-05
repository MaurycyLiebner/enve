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

class UndoRedo : public StdSelfRef {
public:
    UndoRedo(const QString &name);
    virtual ~UndoRedo();
    virtual void undo();
    virtual void redo();
    void printName();
    void printUndoName();
    void printRedoName();

    int getFrame();
private:
    int mFrame;
    QString mName;
};

class UndoRedoSet : public UndoRedo {
public:
    UndoRedoSet();
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
    UndoRedoStack(MainWindow *mainWindow);
    ~UndoRedoStack();

    void startNewSet();
    void finishSet();

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
private:
    void addSet();
    void addToSet(const stdsptr<UndoRedo> &undoRedo);

    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = INT_MAX;
    int mNumberOfSets = 0;

    MainWindow *mMainWindow;
    stdsptr<UndoRedoSet> mCurrentSet;
    QList<stdsptr<UndoRedo>> mUndoStack;
    QList<stdsptr<UndoRedo>> mRedoStack;
};
#endif // UNDOREDO_H
