#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include <QDebug>
#include <memory>
#include <QFont>
#include "skiaincludes.h"

class MainWindow;
class MovablePoint;
typedef QSharedPointer<MovablePoint> MovablePointQSPtr;
class NodePoint;
typedef QSharedPointer<NodePoint> NodePointQSPtr;
class SinglePathAnimator;
typedef QSharedPointer<SinglePathAnimator> SinglePathAnimatorQSPtr;
class VectorPathAnimator;
typedef QSharedPointer<VectorPathAnimator> VectorPathAnimatorQSPtr;
class PathAnimator;
typedef QSharedPointer<PathAnimator> PathAnimatorQSPtr;
class Gradient;
typedef QSharedPointer<Gradient> GradientQSPtr;
class ColorAnimator;
typedef QSharedPointer<ColorAnimator> ColorAnimatorQSPtr;
class PaintSettings;
typedef QSharedPointer<PaintSettings> PaintSettingsQSPtr;
class BoxesGroup;
typedef QSharedPointer<BoxesGroup> BoxesGroupQSPtr;
class BoundingBox;
typedef QSharedPointer<BoundingBox> BoundingBoxQSPtr;
class TextBox;
typedef QSharedPointer<TextBox> TextBoxQSPtr;
class ComplexAnimator;
typedef QSharedPointer<ComplexAnimator> ComplexAnimatorQSPtr;
class Property;
typedef QSharedPointer<Property> PropertyQSPtr;
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;
class Animator;
typedef QSharedPointer<Animator> AnimatorQSPtr;
class Key;
typedef std::shared_ptr<Key> KeyStdPtr;
class QrealKey;
typedef std::shared_ptr<QrealKey> QrealKeyStdPtr;
class QStringAnimator;
typedef QSharedPointer<QStringAnimator> QStringAnimatorQSPtr;

enum CtrlsMode : short;
enum PaintType : short;

class UndoRedo
{
public:
    UndoRedo(QString name);
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

class UndoRedoSet : public UndoRedo
{
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
    UndoRedoStack() {
        startNewSet();
    }

    void startNewSet() {
        mNumberOfSets++;
    }

    ~UndoRedoStack() {
        clearUndoStack();
        clearRedoStack();
    }

    void setWindow(MainWindow *mainWindow);

    void finishSet() {
        mNumberOfSets--;
        if(mNumberOfSets == 0) {
            addSet();
        }
    }

    void addSet() {
        if((mCurrentSet == NULL) ? true : mCurrentSet->isEmpty()) {
            mCurrentSet = NULL;
            return;
        }
        addUndoRedo(mCurrentSet);
        mCurrentSet = NULL;
    }

    void addToSet(UndoRedo *undoRedo) {
        if(mCurrentSet == NULL) {
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

private:
    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = INT_MAX;
    MainWindow *mMainWindow;
    int mNumberOfSets = 0;
    UndoRedoSet *mCurrentSet = NULL;
    QList<UndoRedo*> mUndoStack;
    QList<UndoRedo*> mRedoStack;
};

//class AddPointToSeparatePathsUndoRedo : public UndoRedo
//{
//public:
//    AddPointToSeparatePathsUndoRedo(PathAnimator *path,
//                                    NodePoint *point) :
//        UndoRedo("AddPointToSeparatePathsUndoRedo") {
//        mPath = path;
//        mPoint = point;
//    }

//    ~AddPointToSeparatePathsUndoRedo() {
//    }

//    void redo() {
//        mPath->addPointToSeparatePaths(mPoint, false);
//    }

//    void undo() {
//        mPath->removePointFromSeparatePaths(mPoint, false);
//    }

//private:
//    PathAnimator *mPath;
//    NodePoint *mPoint;
//};

//class RemovePointFromSeparatePathsUndoRedo :
//        public AddPointToSeparatePathsUndoRedo
//{
//public:
//    RemovePointFromSeparatePathsUndoRedo(PathAnimator *path,
//                                         NodePoint *point) :
//        AddPointToSeparatePathsUndoRedo(path, point) {

//    }

//    void redo() {
//        AddPointToSeparatePathsUndoRedo::undo();
//    }

//    void undo() {
//        AddPointToSeparatePathsUndoRedo::redo();
//    }
//};


class MoveChildInListUndoRedo : public UndoRedo {
public:
    MoveChildInListUndoRedo(BoundingBox *child,
                            const int &fromIndex,
                            const int &toIndex,
                            BoxesGroup *parentBox);

    ~MoveChildInListUndoRedo();

    void redo();
    void undo();
private:
    BoxesGroupQSPtr mParentBox;
    BoundingBoxQSPtr mChild;
    int mFromIndex;
    int mToIndex;
};

class MoveChildAnimatorInListUndoRedo : public UndoRedo {
public:
    MoveChildAnimatorInListUndoRedo(Property *child,
                            const int &fromIndex,
                            const int &toIndex,
                            ComplexAnimator *parentAnimator);

    ~MoveChildAnimatorInListUndoRedo();

    void redo();
    void undo();
private:
    ComplexAnimatorQSPtr mParentAnimator;
    PropertyQSPtr mChild;
    int mFromIndex;
    int mToIndex;
};

class SetBoundingBoxZListIndexUnoRedo : public UndoRedo {
public:
    SetBoundingBoxZListIndexUnoRedo(const int &indexBefore,
                                    const int &indexAfter,
                                    BoundingBox *box);

    ~SetBoundingBoxZListIndexUnoRedo();

    void redo();
    void undo();
private:
    int mIndexBefore;
    int mIndexAfter;
    BoundingBoxQSPtr mBox;
};

class AddChildToListUndoRedo : public UndoRedo {
public:
    AddChildToListUndoRedo(BoxesGroup *parent,
                           const int &index,
                           BoundingBox *child);

    ~AddChildToListUndoRedo();

    void redo();
    void undo();
private:
    BoxesGroupQSPtr mParent;
    int mAddAtId;
    BoundingBoxQSPtr mChild;
};

class RemoveChildFromListUndoRedo : public AddChildToListUndoRedo {
public:
    RemoveChildFromListUndoRedo(BoxesGroup *parent,
                                int index,
                                BoundingBox *child) :
        AddChildToListUndoRedo(parent, index, child) {

    }

    void redo() {
        AddChildToListUndoRedo::undo();
    }

    void undo() {
        AddChildToListUndoRedo::redo();
    }
private:
};

class  SetBoxVisibleUndoRedo : public UndoRedo {
public:
     SetBoxVisibleUndoRedo(BoundingBox *target,
                           const bool &visibleBefore,
                           const bool &visibleAfter);

     ~SetBoxVisibleUndoRedo();

     void redo();
     void undo();
private:
     bool mVisibleBefore;
     bool mVisibleAfter;
     BoundingBoxQSPtr mTarget;
};

class ChangeQrealAnimatorValue : public UndoRedo {
public:
    ChangeQrealAnimatorValue(const SkScalar &oldValue,
                             const SkScalar &newValue,
                             QrealAnimator *animator);

    ~ChangeQrealAnimatorValue();

    void redo();
    void undo();
private:
    SkScalar mOldValue;
    SkScalar mNewValue;
    QrealAnimatorQSPtr mAnimator;
};

class ChangeQrealKeyValueUndoRedo : public UndoRedo {
public:
    ChangeQrealKeyValueUndoRedo(const SkScalar &oldValue,
                                const SkScalar &newValue,
                                QrealKey *key);

    ~ChangeQrealKeyValueUndoRedo();

    void redo();
    void undo();
private:
    SkScalar mOldValue;
    SkScalar mNewValue;
    QrealKeyStdPtr mTargetKey;
};

class ChangeKeyFrameUndoRedo : public UndoRedo
{
public:
    ChangeKeyFrameUndoRedo(const int &oldFrame,
                           const int &newFrame,
                           Key *key);

    ~ChangeKeyFrameUndoRedo();

    void redo();
    void undo();
private:
    int mOldFrame;
    int mNewFrame;
    KeyStdPtr mTargetKey;
};

class AnimatorRecordingSetUndoRedo : public UndoRedo {
public:
    AnimatorRecordingSetUndoRedo(const bool &recordingOld,
                                 const bool &recordingNew,
                                 Animator *animator);

    ~AnimatorRecordingSetUndoRedo();

    void undo();
    void redo();
private:
    bool mRecordingOld;
    bool mRecordingNew;
    AnimatorQSPtr mAnimator;
};

class AddKeyToAnimatorUndoRedo : public UndoRedo {
public:
    AddKeyToAnimatorUndoRedo(Key *key, Animator *animator);

    ~AddKeyToAnimatorUndoRedo();

    void redo();
    void undo();
private:
    KeyStdPtr mKey;
    AnimatorQSPtr mAnimator;
};

class RemoveKeyFromAnimatorUndoRedo :
        public AddKeyToAnimatorUndoRedo
{
public:
    RemoveKeyFromAnimatorUndoRedo(Key *key, Animator *animator) :
        AddKeyToAnimatorUndoRedo(key, animator) {

    }

    void redo() {
        AddKeyToAnimatorUndoRedo::undo();
    }

    void undo() {
        AddKeyToAnimatorUndoRedo::redo();
    }
};

class PaintTypeChangeUndoRedo : public UndoRedo {
public:
    PaintTypeChangeUndoRedo(const PaintType &oldType,
                            const PaintType &newType,
                            PaintSettings *target);

    ~PaintTypeChangeUndoRedo();

    void redo();
    void undo();
private:
    PaintType mOldType;
    PaintType mNewType;
    PaintSettingsQSPtr mTarget;
};

class GradientChangeUndoRedo : public UndoRedo {
public:
    GradientChangeUndoRedo(Gradient *oldGradient,
                           Gradient *newGradient,
                           PaintSettings *target);

    ~GradientChangeUndoRedo();

    void redo();
    void undo();
private:
    GradientQSPtr mOldGradient;
    GradientQSPtr mNewGradient;
    PaintSettingsQSPtr mTarget;
};


class GradientColorAddedToListUndoRedo : public UndoRedo {
public:
    GradientColorAddedToListUndoRedo(Gradient *target,
                                     ColorAnimator *color);

    ~GradientColorAddedToListUndoRedo();

    void undo();
    void redo();
private:
    ColorAnimatorQSPtr mColor;
    GradientQSPtr mGradient;
};

class GradientColorRemovedFromListUndoRedo :
        public GradientColorAddedToListUndoRedo {
public:
    GradientColorRemovedFromListUndoRedo(Gradient *target,
                                     ColorAnimator *color) :
        GradientColorAddedToListUndoRedo(target,
                                         color) {
    }

    void undo() {
        GradientColorAddedToListUndoRedo::redo();
    }

    void redo() {
        GradientColorAddedToListUndoRedo::undo();
    }
};

class GradientSwapColorsUndoRedo : public UndoRedo {
public:
    GradientSwapColorsUndoRedo(Gradient *target,
                               const int &id1,
                               const int &id2);

    ~GradientSwapColorsUndoRedo();

    void undo();
    void redo();
private:
    int mId1;
    int mId2;
    GradientQSPtr mGradient;
};

class AddSinglePathAnimatorUndoRedo : public UndoRedo {
public:
    AddSinglePathAnimatorUndoRedo(PathAnimator *target,
                                  VectorPathAnimator *path);

    ~AddSinglePathAnimatorUndoRedo();

    void undo();
    void redo();
private:
    PathAnimatorQSPtr mTarget;
    VectorPathAnimatorQSPtr mPath;
};

class RemoveSinglePathAnimatorUndoRedo :
        public AddSinglePathAnimatorUndoRedo {
public:
    RemoveSinglePathAnimatorUndoRedo(PathAnimator *target,
                                     VectorPathAnimator *path) :
        AddSinglePathAnimatorUndoRedo(target, path) {

    }

    void undo() {
        AddSinglePathAnimatorUndoRedo::redo();
    }

    void redo() {
        AddSinglePathAnimatorUndoRedo::undo();
    }
};

class ChangeFontUndoRedo : public UndoRedo {
public:
    ChangeFontUndoRedo(TextBox *target,
                       const QFont &fontBefore,
                       const QFont &fontAfter);
    ~ChangeFontUndoRedo();

    void undo();
    void redo();
private:
    TextBoxQSPtr mTarget;
    QFont mOldFont;
    QFont mNewFont;
};

class ChangeTextUndoRedo : public UndoRedo {
public:
    ChangeTextUndoRedo(QStringAnimator *target,
                       const QString &textBefore,
                       const QString &textAfter);
    ~ChangeTextUndoRedo();

    void undo();
    void redo();
private:
    QStringAnimatorQSPtr mTarget;
    QString mOldText;
    QString mNewText;
};
#endif // UNDOREDO_H
