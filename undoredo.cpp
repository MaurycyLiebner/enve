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
//        int currentFrame = mMainWindow->getCurrentFrame();
//        if(currentFrame != mLastUndoRedoFrame) {
//            UndoRedo *frameChangeUndoRedo =
//                    new ChangeViewedFrameUndoRedo(mLastUndoRedoFrame,
//                                                  currentFrame);
//            mLastUndoRedoFrame = currentFrame;
//            addUndoRedo(frameChangeUndoRedo);
//        }
//        if(currentFrame != mLastUndoRedoFrame) {
//            bool noSet = mCurrentSet == NULL;

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
    UndoRedo *toRedo = mRedoStack.last();
    if(toRedo->getFrame() != mMainWindow->getCurrentFrame()) {
        mMainWindow->setCurrentFrameForAllWidgets(toRedo->getFrame());
        return;
    }
    mRedoStack.removeLast();
    toRedo->printRedoName();
    toRedo->redo();
    mUndoStack << toRedo;
}

void UndoRedoStack::undo() {
    if(mUndoStack.isEmpty()) {
        return;
    }
    UndoRedo *toUndo = mUndoStack.last();
    if(toUndo->getFrame() != mMainWindow->getCurrentFrame()) {
        mMainWindow->setCurrentFrameForAllWidgets(toUndo->getFrame());
        return;
    }
    mUndoStack.removeLast();
    toUndo->printUndoName();
    toUndo->undo();
    mRedoStack << toUndo;
}

UndoRedo::UndoRedo(QString name) {
    mName = name;
    printName();
    mFrame = MainWindow::getInstance()->getCurrentFrame();
}
