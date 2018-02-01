#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QList>
#include <QDebug>
#include <memory>
#include <QFont>
#include "skiaincludes.h"

#include "Animators/PathAnimators/vectorpathanimator.h"
class MainWindow;
class MovablePoint;
typedef QSharedPointer<MovablePoint> MovablePointQSPtr;
class NodePoint;
typedef QSharedPointer<NodePoint> NodePointQSPtr;
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

    bool undoRedoBlocked() {
        return mUndoRedoBlocked;
    }

    UndoRedo *addAddPointToVectorPathAnimatorUndoRedo(
            VectorPathAnimator *path,
            const QPointF &startRelPos,
            const QPointF &relPos,
            const QPointF &endRelPos,
            const int &targetNodeId,
            const NodeSettings &nodeSettings,
            const int &newNodeId);

    UndoRedo *addMoveChildInListUndoRedo(
            BoundingBox *child,
            const int &fromIndex,
            const int &toIndex,
            BoxesGroup *parentBox);

    UndoRedo *addMoveChildAnimatorInListUndoRedo(
            Property *child,
            const int &fromIndex,
            const int &toIndex,
            ComplexAnimator *parentAnimator);

    UndoRedo *addSetBoundingBoxZListIndexUnoRedo(
            const int &indexBefore,
            const int &indexAfter,
            BoundingBox *box);

    UndoRedo *addAddChildToListUndoRedo(
            BoxesGroup *parent,
            const int &index,
            BoundingBox *child);

    UndoRedo *addRemoveChildFromListUndoRedo(
            BoxesGroup *parent,
            const int &index,
            BoundingBox *child);

    UndoRedo *addSetBoxVisibleUndoRedo(
            BoundingBox *target,
            const bool &visibleBefore,
            const bool &visibleAfter);

    UndoRedo *addChangeQrealAnimatorValue(
            const SkScalar &oldValue,
            const SkScalar &newValue,
            QrealAnimator *animator);

    UndoRedo *addChangeQrealKeyValueUndoRedo(
            const SkScalar &oldValue,
            const SkScalar &newValue,
            QrealKey *key);

    UndoRedo *addChangeKeyFrameUndoRedo(
            const int &oldFrame,
            const int &newFrame,
            Key *key);

    UndoRedo *addAnimatorRecordingSetUndoRedo(
            const bool &recordingOld,
            const bool &recordingNew,
            Animator *animator);

    UndoRedo *addAddKeyToAnimatorUndoRedo(
            Key *key,
            Animator *animator);

    UndoRedo *addRemoveKeyFromAnimatorUndoRedo(
            Key *key,
            Animator *animator);

    UndoRedo *addPaintTypeChangeUndoRedo(
            const PaintType &oldType,
            const PaintType &newType,
            PaintSettings *target);

    UndoRedo *addGradientChangeUndoRedo(
            Gradient *oldGradient,
            Gradient *newGradient,
            PaintSettings *target);

    UndoRedo *addGradientColorAddedToListUndoRedo(
            Gradient *target,
            ColorAnimator *color);

    UndoRedo *addGradientColorRemovedFromListUndoRedo(
            Gradient *target,
            ColorAnimator *color);

    UndoRedo *addGradientSwapColorsUndoRedo(
            Gradient *target,
            const int &id1,
            const int &id2);

    UndoRedo *addAddSinglePathAnimatorUndoRedo(
            PathAnimator *target,
            VectorPathAnimator *path);

    UndoRedo *addRemoveSinglePathAnimatorUndoRedo(
            PathAnimator *target,
            VectorPathAnimator *path);

    UndoRedo *addChangeFontUndoRedo(
            TextBox *target,
            const QFont &fontBefore,
            const QFont &fontAfter);

    UndoRedo *addChangeTextUndoRedo(
            QStringAnimator *target,
            const QString &textBefore,
            const QString &textAfter);

    UndoRedo *addPathContainerAddNodeElementsUR(
            PathContainer *target,
            const int &startPtIndex,
            const SkPoint &startPos,
            const SkPoint &pos,
            const SkPoint &endPos);

    UndoRedo *addPathContainerdRemoveNodeElementsUR(
            PathContainer *target,
            const int &startPtIndex,
            const SkPoint &startPos,
            const SkPoint &pos,
            const SkPoint &endPos);

    UndoRedo *addPathContainerPathChangeUR(
            PathContainer *target,
            const QList<SkPoint> oldPts,
            const QList<SkPoint> newPts);

    UndoRedo *addVectorPathAnimatorReplaceNodeSettingsUR(
            VectorPathAnimator *target,
            const int &index,
            const NodeSettings &oldSettings,
            const NodeSettings &settings);

    UndoRedo *addVectorPathAnimatorInsertNodeSettingsUR(
            VectorPathAnimator *target,
            const int &index,
            const NodeSettings &settings);
    UndoRedo *addVectorPathAnimatorRemoveNodeSettingsUR(
            VectorPathAnimator *target,
            const int &index,
            const NodeSettings &settings);
private:
    bool mUndoRedoBlocked = false;
    int mLastUndoRedoFrame = INT_MAX;
    MainWindow *mMainWindow;
    int mNumberOfSets = 0;
    UndoRedoSet *mCurrentSet = NULL;
    QList<UndoRedo*> mUndoStack;
    QList<UndoRedo*> mRedoStack;
};

class AddPointToVectorPathAnimatorUndoRedo : public UndoRedo {
public:
    AddPointToVectorPathAnimatorUndoRedo(VectorPathAnimator *path,
                                         const QPointF &startRelPos,
                                         const QPointF &relPos,
                                         const QPointF &endRelPos,
                                         const int &targetNodeId,
                                         const NodeSettings &nodeSettings,
                                         const int &newNodeId);

    ~AddPointToVectorPathAnimatorUndoRedo();

    void redo();

    void undo();

private:
    VectorPathAnimator *mPath;
    QPointF mStartRelPos;
    QPointF mRelPos;
    QPointF mEndRelPos;
    int mTargetNodeId;
    int mNewPointId;
    NodeSettings mNodeSetting;
};

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

class SetBoxVisibleUndoRedo : public UndoRedo {
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

class PathContainerAddNodeElementsUR : public UndoRedo {
public:
    PathContainerAddNodeElementsUR(PathContainer *target,
                                   const int &startPtIndex,
                                   const SkPoint &startPos,
                                   const SkPoint &pos,
                                   const SkPoint &endPos) :
        UndoRedo("PathContainerAddNodeElementsUR") {
        mTarget = target;
        mStartPtIndex = startPtIndex;
        mStartPos = startPos;
        mPos = pos;
        mEndPos = endPos;
    }
    ~PathContainerAddNodeElementsUR() {}

    void undo() {
        mTarget->removeNodeElements(mStartPtIndex, false);
        mTarget->updateAfterChangedFromInside();
    }

    void redo() {
        mTarget->addNodeElements(mStartPtIndex,
                                 mStartPos,
                                 mPos,
                                 mEndPos, false);
        mTarget->updateAfterChangedFromInside();
    }
private:
    PathContainer *mTarget;
    int mStartPtIndex;
    SkPoint mStartPos;
    SkPoint mPos;
    SkPoint mEndPos;
};

class PathContainerdRemoveNodeElementsUR : public UndoRedo {
public:
    PathContainerdRemoveNodeElementsUR(PathContainer *target,
                                       const int &startPtIndex,
                                       const SkPoint &startPos,
                                       const SkPoint &pos,
                                       const SkPoint &endPos) :
        UndoRedo("PathContainerdRemoveNodeElementsUR") {
        mTarget = target;
        mStartPtIndex = startPtIndex;
        mStartPos = startPos;
        mPos = pos;
        mEndPos = endPos;
    }
    ~PathContainerdRemoveNodeElementsUR() {}

    void undo() {
        mTarget->addNodeElements(mStartPtIndex,
                                 mStartPos,
                                 mPos,
                                 mEndPos, false);
        mTarget->updateAfterChangedFromInside();
    }

    void redo() {
        mTarget->removeNodeElements(mStartPtIndex, false);
        mTarget->updateAfterChangedFromInside();
    }
private:
    PathContainer *mTarget;
    int mStartPtIndex;
    SkPoint mStartPos;
    SkPoint mPos;
    SkPoint mEndPos;
};

class PathContainerPathChangeUR : public UndoRedo {
public:
    PathContainerPathChangeUR(PathContainer *target,
                              const QList<SkPoint> oldPts,
                              const QList<SkPoint> newPts) :
    UndoRedo("PathContainerPathChangeUR") {
        mTarget = target;
        mOldPts = oldPts;
        mNewPts = newPts;
    }
    ~PathContainerPathChangeUR() {}

    void undo() {
        mTarget->setElementsPos(mOldPts, false);
        mTarget->updateAfterChangedFromInside();
    }

    void redo() {
        mTarget->setElementsPos(mNewPts, false);
        mTarget->updateAfterChangedFromInside();
    }
private:
    PathContainer *mTarget;
    QList<SkPoint> mOldPts;
    QList<SkPoint> mNewPts;
};

class VectorPathAnimatorReplaceNodeSettingsUR : public UndoRedo {
public:
    VectorPathAnimatorReplaceNodeSettingsUR(
             VectorPathAnimator *target,
             const int &index,
             const NodeSettings &oldSettings,
             const NodeSettings &settings) :
    UndoRedo("VectorPathAnimatorReplaceNodeSettingsUR") {
        mTarget = target;
        mId = index;
        mOldSettings = oldSettings;
        mSettings = settings;
    }
    ~VectorPathAnimatorReplaceNodeSettingsUR() {}

    void undo() {
        mTarget->replaceNodeSettingsForNodeId(mId, mOldSettings, false);
    }

    void redo() {
        mTarget->replaceNodeSettingsForNodeId(mId, mSettings, false);
    }
private:
    VectorPathAnimator *mTarget;
    int mId;
    NodeSettings mOldSettings;
    NodeSettings mSettings;
};

class VectorPathAnimatorInsertNodeSettingsUR : public UndoRedo {
public:
    VectorPathAnimatorInsertNodeSettingsUR(VectorPathAnimator *target,
                                 const int &index,
                                 const NodeSettings &settings) :
    UndoRedo("VectorPathAnimatorInsertNodeSettingsUR") {
        mTarget = target;
        mId = index;
        mSettings = settings;
    }
    ~VectorPathAnimatorInsertNodeSettingsUR() {}

    void undo() {
        mTarget->removeNodeSettingsAt(mId, false);
    }

    void redo() {
        mTarget->insertNodeSettingsForNodeId(mId, mSettings, false);
    }
private:
    VectorPathAnimator *mTarget;
    int mId;
    NodeSettings mSettings;
};

class VectorPathAnimatorRemoveNodeSettingsUR : public UndoRedo {
public:
    VectorPathAnimatorRemoveNodeSettingsUR(VectorPathAnimator *target,
                                 const int &index,
                                 const NodeSettings &settings) :
    UndoRedo("VectorPathAnimatorRemoveNodeSettingsUR") {
        mTarget = target;
        mId = index;
        mSettings = settings;
    }
    ~VectorPathAnimatorRemoveNodeSettingsUR() {}

    void undo() {
        mTarget->insertNodeSettingsForNodeId(mId, mSettings, false);
    }

    void redo() {
        mTarget->removeNodeSettingsAt(mId, false);
    }
private:
    VectorPathAnimator *mTarget;
    int mId;
    NodeSettings mSettings;
};

class TileChangedUndoRedo : public UndoRedo {
public:
    TileChangedUndoRedo(Tile *target) :
        UndoRedo("TileChangedUndoRedo") {
        mTarget = target;
    }

    void setOldBitmap(const SkBitmap &bitmap) {
        SkPixmap pix;
        bitmap.peekPixels(&pix);
        mOldImage = SkImage::MakeRasterCopy(pix);
    }

    void setNewBitmap(const SkBitmap &bitmap) {
        SkPixmap pix;
        bitmap.peekPixels(&pix);
        mNewImage = SkImage::MakeRasterCopy(pix);
    }
protected:
    SkBitmap mOldImage;
    SkBitmap mNewImage;
    Tile *mTarget;
};
#endif // UNDOREDO_H
