#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>
#include <QPainter>
#include <QMouseEvent>
#include "float.h"
#include "Animators/graphanimator.h"
class QrealKey;
class QrealPoint;
class QrealAnimatorValueSlider;
class RandomQrealGenerator;

#include <QDoubleSpinBox>

class QrealAnimator :  public GraphAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    virtual ~QrealAnimator();

    void getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveValue, qreal &maxMoveValue) const {
        Q_UNUSED(key);
        Q_UNUSED(type);
        minMoveValue = mMinPossibleVal;
        maxMoveValue = mMaxPossibleVal;
    }

    void qra_setValueRange(const qreal &minVal,
                           const qreal &maxVal);

    qreal qra_getValueAtAbsFrame(const int &frame) const;
    qreal qra_getCurrentValue() const;
    qreal qra_getCurrentEffectiveValue();
    void qra_setCurrentValue(qreal newValue,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false,
                             const bool &callUpdater = true);
    void qra_updateValueFromCurrentFrame();
    void qra_saveCurrentValueToKey(QrealKey *key,
                                   const bool &finish = true);
    void qra_saveValueToKey(QrealKey *key,
                            const qreal &value,
                            const bool &saveUndoRedo = true,
                            const bool &finish = true);

    void prp_setAbsFrame(const int &frame);

    qreal qra_getValueAtRelFrame(const int &frame,
                                 QrealKey *prevKey,
                                 QrealKey *nextKey) const;

    qreal clampGraphValue(const qreal &value);

    qreal qra_getPrevKeyValue(const QrealKey * const key);
    qreal qra_getNextKeyValue(const QrealKey * const key);
    int qra_getPrevKeyRelFrame(const QrealKey * const key);
    int qra_getNextKeyRelFrame(const QrealKey * const key);

    virtual void prp_retrieveSavedValue();
    void qra_incCurrentValue(const qreal &incBy,
                             const bool &saveUndoRedo = false,
                             const bool &finish = false,
                             const bool &callUpdater = true);

    virtual void prp_startTransform();

    void prp_finishTransform();

    void qra_multCurrentValue(const qreal &mult);

    qreal qra_getSavedValue();

    virtual void prp_cancelTransform();  

    void qra_incAllValues(const qreal &valInc,
                          const bool &saveUndoRedo = false,
                          const bool &finish = false,
                          const bool &callUpdater = true);

    virtual QString prp_getValueText();

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    void anim_mergeKeysIfNeeded();
    void anim_appendKey(const stdsptr<Key> &newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);
    void anim_removeKey(const stdsptr<Key> &keyToRemove,
                        const bool &saveUndoRedo = true);
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo,
                                const bool &finish);

    qreal getPrefferedValueStep();
    virtual void setPrefferedValueStep(const qreal &valueStep);
    virtual void prp_clearFromGraphView();

    void graphFixMinMaxValues();
    void saveValueAtAbsFrameAsKey(const int &frame);

    virtual void prp_openContextMenu(const QPoint &pos);
    void qra_saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal &incBy);
    void multSavedValueToCurrentValue(const qreal &multBy);

    const int &getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int &decimals) { mDecimals = decimals; }

    qreal getCurrentValueAtAbsFrame(const int &frame);
    qreal getCurrentValueAtRelFrame(const int &frame) const;
    qreal qra_getValueAtRelFrame(const int &frame) const;
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
    bool getBeingTransformed() { return mTransformed; }
    void anim_removeAllKeys();

    void prp_updateAfterChangedRelFrameRange(const int &minFrame,
                                             const int &maxFrame) {
        if(anim_mCurrentRelFrame >= minFrame) {
            if(anim_mCurrentRelFrame <= maxFrame) {
                qra_updateValueFromCurrentFrame();
            }
        }
        Property::prp_updateAfterChangedRelFrameRange(minFrame, maxFrame);
    }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

    bool SWT_isQrealAnimator() { return true; }
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    stdsptr<Key> readKey(QIODevice *target);
    bool qra_hasNoise();

    qreal qra_getEffectiveValueAtRelFrame(const int &frame) const;
    qreal qra_getEffectiveValueAtAbsFrame(const int &frame) const;
    qreal getCurrentEffectiveValueAtRelFrame(const int &frame) const;
    qreal getCurrentEffectiveValueAtAbsFrame(const int &frame);
    qreal qra_getValueAtRelFrameF(const qreal &frame) const;
    qreal qra_getEffectiveValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentEffectiveValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentEffectiveValueAtAbsFrameF(const qreal &frame);
    qreal qra_getValueAtRelFrameF(const qreal &frame,
                                  QrealKey *prevKey,
                                  QrealKey *nextKey) const;
    qreal getCurrentValueAtRelFrameF(const qreal &frame) const;
    qreal getCurrentValueAtAbsFrameF(const qreal &frame);

    static auto create0to1Animator(const QString& name) {
        auto anim = SPtrCreate(QrealAnimator)(0., 0., 1., 0.01, name);
        anim->graphFixMinMaxValues();
        return anim;
    }

    void anim_updateKeysPath();
    void anim_getMinAndMaxValues(qreal &minValP, qreal &maxValP) const;
    void anim_getMinAndMaxValuesBetweenFrames(const int &startFrame,
                                              const int &endFrame,
                                              qreal &minValP,
                                              qreal &maxValP) const;
    bool anim_graphValuesCorrespondToFrames() const {
        return false;
    }
protected:
    QrealAnimator(const QString& name);
    QrealAnimator(const qreal &iniVal,
                  const qreal &minVal,
                  const qreal &maxVal,
                  const qreal &prefferdStep,
                  const QString& name);
    QrealKey *getQrealKeyAtId(const int &id) const;

    bool mGraphMinMaxValuesFixed = false;
    bool mTransformed = false;

    int mDecimals = 3;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = -DBL_MAX;

    qreal mCurrentValue = 0.;
    qreal mSavedCurrentValue = 0.;

    qsptr<RandomQrealGenerator> mRandomGenerator;

    qreal mPrefferedValueStep = 1.;
signals:
    void valueChangedSignal(qreal);
public slots:
    void prp_setRecording(const bool& rec);

    void anim_saveCurrentValueAsKey();
};

#endif // VALUEANIMATORS_H
