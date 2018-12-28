#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "Animators/animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboardContainer;
class QrealAnimator;

class ComplexAnimator : public Animator {
    Q_OBJECT
    friend class SelfRef;
public:
    ComplexAnimator(const QString& name);
    ~ComplexAnimator();

    void ca_addChildAnimator(const qsptr<Property> &childAnimator,
                             const int &id = INT_MAX);
    void ca_removeChildAnimator(const qsptr<Property> &removeAnimator);
    void ca_swapChildAnimators(Property *animator1, Property *animator2);
    void ca_moveChildInList(Property *child,
                            const int &from,
                            const int &to,
                            const bool &saveUndoRedo = true);
    void ca_moveChildBelow(Property *move,
                           Property *below);
    void ca_moveChildAbove(Property *move,
                           Property *above,
                           const bool &saveUndoRedo = true);

    void prp_startTransform();
    void prp_setUpdater(const stdsptr<PropertyUpdater>& updater);
    void prp_setAbsFrame(const int &frame);

    void prp_retrieveSavedValue();
    void prp_finishTransform();
    void prp_cancelTransform();

    bool prp_isDescendantRecording();
    QString prp_getValueText();
    void prp_clearFromGraphView();

    bool hasChildAnimators();

    void prp_setTransformed(const bool &bT);

    void ca_changeChildAnimatorZ(const int &oldIndex,
                                 const int &newIndex);
    int ca_getNumberOfChildren();
    Property *ca_getChildAt(const int &i);

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_isComplexAnimator();

    void anim_drawKey(QPainter *p,
                      Key* key,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &rowHeight,
                      const int &keyRectSize);


    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator = nullptr);
    int getChildPropertyIndex(Property *child);

    void ca_updateDescendatKeyFrame(Key* key);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    void anim_saveCurrentValueAsKey();
    virtual void ca_removeAllChildAnimators();
    Property *ca_getFirstDescendantWithName(const QString &name);

    template <class T = Property>
    T *getPropertyIfIsTheOnlyOne(bool (Property::*tester)()) {
        if(ca_mChildAnimators.count() == 1) {
            Property* prop = ca_mChildAnimators.first().get();
            if((prop->*tester)()) {
                return static_cast<T*>(prop);
            }
        }
        return nullptr;
    }

    void SWT_setChildrenAncestorDisabled(const bool &bT) {
        Q_FOREACH(const qsptr<Property> &prop, ca_mChildAnimators) {
            prop->SWT_setAncestorDisabled(bT);
        }
    }
public slots:
    void ca_prependChildAnimator(Property *childAnimator,
                                 const qsptr<Property>& prependWith);
    void ca_replaceChildAnimator(const qsptr<Property> &childAnimator,
                                 const qsptr<Property>& replaceWith);
    void prp_setRecording(const bool &rec);

    void ca_addDescendantsKey(Key * const key);
    void ca_removeDescendantsKey(Key * const key);
    virtual void ca_childAnimatorIsRecordingChanged();
protected:
    ComplexKey *ca_getKeyCollectionAtAbsFrame(const int &frame);
    ComplexKey *ca_getKeyCollectionAtRelFrame(const int &frame);
    bool ca_mChildAnimatorRecording = false;
    QList<qsptr<Property>> ca_mChildAnimators;
};

class ComplexKey : public Key {
    friend class StdSelfRef;
public:
    void addAnimatorKey(Key * const key);

    void addOrMergeKey(const stdsptr<Key> &keyAdd);

    void deleteKey();

    void removeAnimatorKey(Key * const key);

    bool isEmpty() const;

    void setRelFrame(const int &frame);

    void mergeWith(const stdsptr<Key> &key);

    void margeAllKeysToKey(ComplexKey * const target);

    bool isDescendantSelected() const;

    void startFrameTransform();
    void finishFrameTransform();
    void cancelFrameTransform();

    bool isSelected() const;
    void addToSelection(QList<qptr<Animator>> &selectedAnimators);
    void removeFromSelection(QList<qptr<Animator>> &selectedAnimators);

    bool hasKey(Key *key) const;

    bool differsFromKey(Key *otherKey) const;

    int getChildKeysCount() const;

    bool hasSameKey(Key *otherKey) const;

    qreal getEndValueForGraph() const {
        int nAvg = 0;
        qreal sum = 0.;
        Q_FOREACH(const auto& key, mKeys) {
            if(!key->getEndEnabledForGraph()) continue;
            nAvg++;
            sum += key->getEndValueDirectionForGraph();
        }
        if(nAvg == 0) return mRelFrame;
        return getEndValueForGraphEndValueDirection(sum/nAvg);
    }

    qreal getStartValueForGraph() const {
        int nAvg = 0;
        qreal sum = 0.;
        Q_FOREACH(const auto& key, mKeys) {
            if(!key->getStartEnabledForGraph()) continue;
            nAvg++;
            sum += key->getStartValueDirectionForGraph();
        }
        if(nAvg == 0) return mRelFrame;
        return getStartValueForGraphStartValueDirection(sum/nAvg);
    }

    qreal getStartFrameForGraph() const {
        int nAvg = 0;
        qreal sum = 0.;
        Q_FOREACH(const auto& key, mKeys) {
            if(!key->getStartEnabledForGraph()) continue;
            nAvg++;
            sum += key->getStartFrameForGraph();
        }
        if(nAvg == 0) return mRelFrame;
        return sum/nAvg;
    }

    qreal getEndFrameForGraph() const {
        int nAvg = 0;
        qreal sum = 0.;
        Q_FOREACH(const auto& key, mKeys) {
            if(!key->getEndEnabledForGraph()) continue;
            nAvg++;
            sum += key->getEndFrameForGraph();
        }
        if(nAvg == 0) return mRelFrame;
        return sum/nAvg;
    }

    bool getStartEnabledForGraph() const {
        Q_FOREACH(const auto& key, mKeys) {
            if(key->getStartEnabledForGraph()) return true;
        }
        return false;
    }

    bool getEndEnabledForGraph() const {
        Q_FOREACH(const auto& key, mKeys) {
            if(key->getEndEnabledForGraph()) return true;
        }
        return false;
    }

    void setEndValueDirectionForGraph(const qreal &value) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setEndValueDirectionForGraph(value);
        }
    }

    void setStartValueDirectionForGraph(const qreal &value) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setStartValueDirectionForGraph(value);
        }
    }

    void setEndValueForGraph(const qreal& value) {
        qreal dir = getEndValueDirectionForGraphForEndValue(value);
        Q_FOREACH(const auto& key, mKeys) {
            key->setEndValueDirectionForGraph(dir);
        }
    }

    void setStartValueForGraph(const qreal& value) {
        qreal dir = getStartValueDirectionForGraphForStartValue(value);
        Q_FOREACH(const auto& key, mKeys) {
            key->setStartValueDirectionForGraph(dir);
        }
    }

    void setStartFrameForGraph(const qreal& frame) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setStartFrameForGraph(frame);
        }
    }

    void setEndFrameForGraph(const qreal& frame) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setEndFrameForGraph(frame);
        }
    }

    void setStartEnabledForGraph(const bool& enabled) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setStartEnabledForGraph(enabled);
        }
    }

    void setEndEnabledForGraph(const bool& enabled) {
        Q_FOREACH(const auto& key, mKeys) {
            key->setEndEnabledForGraph(enabled);
        }
    }
protected:
    ComplexKey(ComplexAnimator* parentAnimator);
private:
    QList<stdptr<Key>> mKeys;
};

#endif // COMPLEXANIMATOR_H
