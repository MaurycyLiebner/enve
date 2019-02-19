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
    ~QrealAnimator();

    void anim_setAbsFrame(const int &frame);
    void prp_retrieveSavedValue();
    void prp_startTransform();
    void prp_finishTransform();
    void prp_cancelTransform();
    QString prp_getValueText();
    void prp_clearFromGraphView();
    void prp_openContextMenu(const QPoint &pos);
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
    void prp_updateAfterChangedRelFrameRange(const FrameRange& range) {
        if(range.inRange(anim_mCurrentRelFrame)) {
            qra_updateValueFromCurrentFrame();
        }
        Animator::prp_updateAfterChangedRelFrameRange(range);
    }

    void anim_mergeKeysIfNeeded();
    void anim_appendKey(const stdsptr<Key> &newKey);
    void anim_removeKey(const stdsptr<Key> &keyToRemove);
    void anim_moveKeyToRelFrame(Key *key, const int &newFrame);
    void anim_removeAllKeys();
    void graph_updateKeysPath();
    void graph_getMinAndMaxValues(qreal &minValP, qreal &maxValP) const;
    void graph_getMinAndMaxValuesBetweenFrames(const int &startFrame,
                                              const int &endFrame,
                                              qreal &minValP,
                                              qreal &maxValP) const;
    bool graph_graphValuesCorrespondToFrames() const {
        return false;
    }

    qreal graph_clampGraphValue(const qreal &value);

    bool SWT_isQrealAnimator() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    stdsptr<Key> readKey(QIODevice *target);
public slots:
    void anim_setRecording(const bool& rec);
    void anim_saveCurrentValueAsKey();
public:
    void setPrefferedValueStep(const qreal &valueStep);

    void qra_setValueRange(const qreal &minVal,
                           const qreal &maxVal);

    qreal qra_getValueAtAbsFrame(const int &frame) const;
    qreal qra_getCurrentValue() const;
    qreal qra_getCurrentEffectiveValue();
    void qra_setCurrentValue(qreal newValue);
    void qra_updateValueFromCurrentFrame();
    void qra_saveCurrentValueToKey(QrealKey *key);
    void qra_saveValueToKey(QrealKey *key, const qreal &value);

    qreal qra_getValueAtRelFrame(const int &frame,
                                 QrealKey *prevKey,
                                 QrealKey *nextKey) const;


    qreal qra_getPrevKeyValue(const QrealKey * const key);
    qreal qra_getNextKeyValue(const QrealKey * const key);
    int qra_getPrevKeyRelFrame(const QrealKey * const key);
    int qra_getNextKeyRelFrame(const QrealKey * const key);

    void qra_incCurrentValue(const qreal &incBy);

    void qra_multCurrentValue(const qreal &mult);

    qreal qra_getSavedValue();

    void qra_incAllValues(const qreal &valInc);

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    qreal getPrefferedValueStep();

    void graphFixMinMaxValues();
    void saveValueAtAbsFrameAsKey(const int &frame);

    void qra_saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal &incBy);
    void multSavedValueToCurrentValue(const qreal &multBy);

    const int &getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int &decimals) { mDecimals = decimals; }

    qreal getCurrentValueAtAbsFrame(const int &frame);
    qreal getCurrentValueAtRelFrame(const int &frame) const;
    qreal qra_getValueAtRelFrame(const int &frame) const;

    bool getBeingTransformed() { return mTransformed; }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

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
protected:
    QrealAnimator(const QString& name);
    QrealAnimator(const qreal &iniVal,
                  const qreal &minVal,
                  const qreal &maxVal,
                  const qreal &prefferdStep,
                  const QString& name);
    QrealKey *getQrealKeyAtId(const int &id) const;
    void graph_getValueConstraints(GraphKey *key, const QrealPointType& type,
                             qreal &minMoveValue, qreal &maxMoveValue) const;

    bool mGraphMinMaxValuesFixed = false;
    bool mTransformed = false;

    int mDecimals = 3;

    qreal mMaxPossibleVal = DBL_MAX;
    qreal mMinPossibleVal = DBL_MIN;

    qreal mCurrentValue = 0;
    qreal mSavedCurrentValue = 0;

    qsptr<RandomQrealGenerator> mRandomGenerator;

    qreal mPrefferedValueStep = 1;
signals:
    void valueChangedSignal(qreal);
};

#endif // VALUEANIMATORS_H
