#include "undoredo.h"
#include "mainwindow.h"

void UndoRedoStack::setWindow(MainWindow *mainWindow) {
    mMainWindow = mainWindow;
}

void UndoRedoStack::addUndoRedo(UndoRedo *undoRedo) {
    mMainWindow->setFileChangedSinceSaving(true);
    if(mNumberOfSets != 0) {
        addToSet(undoRedo);
    } else {
        clearRedoStack();
        emptySomeOfUndo();
        mUndoStack << undoRedo;
    }
}
