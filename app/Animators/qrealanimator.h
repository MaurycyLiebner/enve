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

    bool SWT_isQrealAnimator() const { return true; }
    void prp_retrieveSavedValue();
    void prp_startTransform();
    void prp_finishTransform();
    void prp_cancelTransform();
    QString prp_getValueText();
    void prp_openContextMenu(const QPoint &pos);
    void prp_setTransformed(const bool &bT) { mTransformed = bT; }
    void prp_updateAfterChangedRelFrameRange(const FrameRange& range) {
        if(range.inRange(anim_getCurrentRelFrame()))
            updateBaseValueFromCurrentFrame();
        GraphAnimator::prp_updateAfterChangedRelFrameRange(range);
    }

    void anim_setAbsFrame(const int &frame);
    void anim_removeAllKeys();
    void anim_addKeyAtRelFrame(const int& relFrame);

    void graph_updateKeyPathWithId(const int &id);

    ValueRange graph_getMinAndMaxValues() const;
    ValueRange graph_getMinAndMaxValuesBetweenFrames(
            const int &startFrame, const int &endFrame) const;

    qreal graph_clampGraphValue(const qreal &value);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    stdsptr<Key> readKey(QIODevice *target);
public slots:
    void anim_saveCurrentValueAsKey();
protected:
    void graph_getValueConstraints(GraphKey *key, const QrealPointType& type,
                                   qreal &minMoveValue, qreal &maxMoveValue) const;
public:
    void setPrefferedValueStep(const qreal &valueStep);

    void setValueRange(const qreal &minVal, const qreal &maxVal);

    void setCurrentBaseValue(qreal newValue);
    void setCurrentBaseValueNoUpdate(const qreal &newValue);
    void incCurrentBaseValue(const qreal &incBy);
    void incCurrentValueNoUpdate(const qreal &incBy);
    void multCurrentBaseValue(const qreal &mult);

    qreal getCurrentBaseValue() const;
    qreal getCurrentEffectiveValue() const;
    qreal getBaseValueAtRelFrame(const qreal &frame) const;
    qreal getBaseValueAtAbsFrame(const qreal &frame) const;
    qreal getEffectiveValueAtRelFrame(const qreal &frame) const;
    qreal getEffectiveValueAtAbsFrame(const qreal &frame) const;

    qreal getSavedBaseValue();
    void incAllValues(const qreal &valInc);

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    qreal getPrefferedValueStep();

    void graphFixMinMaxValues();

    void saveValueToKey(const int &frame, const qreal &value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal &incBy);
    void multSavedValueToCurrentValue(const qreal &multBy);

    const int &getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int &decimals) { mDecimals = decimals; }

    bool getBeingTransformed() { return mTransformed; }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

    bool hasNoise();

    static auto create0to1Animator(const QString& name) {
        auto anim = SPtrCreate(QrealAnimator)(0, 0, 1, 0.01, name);
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
private:
    qreal calculateBaseValueAtRelFrame(const qreal &frame) const;
    QrealKey *getQrealKeyAtId(const int &id) const;

    bool mGraphMinMaxValuesFixed = false;
    bool mTransformed = false;

    int mDecimals = 3;

    qreal mMaxPossibleVal = TEN_MIL;
    qreal mMinPossibleVal = -TEN_MIL;

    qreal mCurrentBaseValue = 0;
    qreal mSavedCurrentValue = 0;

    qsptr<RandomQrealGenerator> mRandomGenerator;

    qreal mPrefferedValueStep = 1;
    bool updateBaseValueFromCurrentFrame();
signals:
    void valueChangedSignal(qreal);
};

#endif // VALUEANIMATORS_H
