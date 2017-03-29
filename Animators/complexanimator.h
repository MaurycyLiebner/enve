#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "Animators/qrealanimator.h"

class ComplexKey;
class ComplexAnimatorItemWidgetContainer;

class ComplexAnimator : public Animator
{
public:
    ComplexAnimator();
    ~ComplexAnimator();

    void ca_addDescendantsKey(Key *key);

    void ca_removeDescendantsKey(Key *key);

    void ca_addChildAnimator(Property *childAnimator);
    void ca_removeChildAnimator(Property *removeAnimator);
    void ca_swapChildAnimators(Property *animator1, Property *animator2);
    void ca_moveChildInList(Property *child,
                         int from, int to,
                         bool saveUndoRedo = true);
    void ca_moveChildBelow(Property *move, Property *below);
    void ca_moveChildAbove(Property *move, Property *above);

    void prp_startTransform();
    void prp_setUpdater(AnimatorUpdater *updater);
    void prp_setAbsFrame(int frame);

    void prp_retrieveSavedValue();
    void prp_finishTransform();
    void prp_cancelTransform();

    void prp_setRecording(bool rec);

    virtual void ca_childAnimatorIsRecordingChanged();
    void ca_setRecordingValue(bool rec);

    bool prp_isDescendantRecording();
    QString prp_getValueText();
    void prp_clearFromGraphView();

    bool hasChildAnimators();

    void prp_setTransformed(bool bT);

    void ca_changeChildAnimatorZ(const int &oldIndex,
                              const int &newIndex);
    int ca_getNumberOfChildren();
    Property *ca_getChildAt(const int &i);

    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) {
        if(hasChildAnimators()) {
            return Animator::SWT_shouldBeVisible(
                                        rules,
                                        parentSatisfies,
                                        parentMainTarget);
        } else {
            return false;
        }
    }

    SWT_Type SWT_getType() { return SWT_ComplexAnimator; }
    void anim_drawKey(QPainter *p, Key *key,
                 const qreal &pixelsPerFrame,
                 const qreal &drawY, const int &startFrame);

    void anim_loadKeysFromSql(int qrealAnimatorId) {
        Q_UNUSED(qrealAnimatorId);
    }

    void prp_makeDuplicate(Property *) {}
    Property *prp_makeDuplicate() {}
    int prp_saveToSql(QSqlQuery *, const int &) {}
    void prp_loadFromSql(const int &) {}
protected:
    ComplexKey *ca_getKeyCollectionAtAbsFrame(int frame);
    ComplexKey *ca_getKeyCollectionAtRelFrame(int frame);
    bool ca_mChildAnimatorRecording = false;
    QList<Property*> ca_mChildAnimators;
};

class ComplexKey : public Key
{
public:
    ComplexKey(ComplexAnimator *parentAnimator);

    void addAnimatorKey(Key* key);

    void addOrMergeKey(Key *keyAdd);

    void deleteKey();

    void removeAnimatorKey(Key *key);

    bool isEmpty();

    void setRelFrame(int frame);

    void mergeWith(Key *key);

    void margeAllKeysToKey(ComplexKey *target);

    bool isDescendantSelected();

    void startFrameTransform();
    void finishFrameTransform();
    void copyToContainer(KeysClipboardContainer *container);
    void cancelFrameTransform();
    //void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame, const qreal &scaleFactor);
    //QrealKey *makeQrealKeyDuplicate(QrealAnimator *targetParent);

    bool areAllChildrenSelected() {
        foreach(Key *key, mKeys) {
            if(key->isSelected() ||
               key->areAllChildrenSelected()) continue;
            return false;
        }

        return true;
    }
    void removeFromSelection(QList<Key *> *selectedKeys);
    void addToSelection(QList<Key *> *selectedKeys);
private:
    QList<Key*> mKeys;
};

#endif // COMPLEXANIMATOR_H
