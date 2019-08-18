#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include "graphanimator.h"
#include "qrealsnapshot.h"
class QrealKey;
class QrealPoint;
class RandomQrealGenerator;

class QrealAnimator :  public GraphAnimator {
    Q_OBJECT
    e_OBJECT
protected:
    QrealAnimator(const QString& name);
    QrealAnimator(const qreal iniVal,
                  const qreal minVal,
                  const qreal maxVal,
                  const qreal prefferdStep,
                  const QString& name);
public:
    bool SWT_isQrealAnimator() const { return true; }
    void prp_startTransform();
    void prp_finishTransform();
    void prp_cancelTransform();
    QString prp_getValueText();
    void prp_setTransformed(const bool bT) { mTransformed = bT; }
    void prp_afterChangedAbsRange(const FrameRange& range,
                                  const bool clip = true) {
        if(range.inRange(anim_getCurrentAbsFrame()))
            updateBaseValueFromCurrentFrame();
        GraphAnimator::prp_afterChangedAbsRange(range, clip);
    }

    void anim_setAbsFrame(const int frame);
    void anim_removeAllKeys();
    void anim_addKeyAtRelFrame(const int relFrame);

    QPainterPath graph_getPathForSegment(
            const GraphKey * const prevKey,
            const GraphKey * const nextKey) const;

    qValueRange graph_getMinAndMaxValues() const;
    qValueRange graph_getMinAndMaxValuesBetweenFrames(
            const int startFrame, const int endFrame) const;

    qreal graph_clampGraphValue(const qreal value);

    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
    stdsptr<Key> createKey();
    void anim_saveCurrentValueAsKey();
protected:
    void graph_getValueConstraints(GraphKey *key, const QrealPointType& type,
                                   qreal &minMoveValue, qreal &maxMoveValue) const;
public:
    QrealSnapshot makeSnapshot(const qreal frameMultiplier = 1,
                          const qreal valueMultiplier = 1) const;
    QrealSnapshot makeSnapshot(const int minFrame, const int maxFrame,
                          const qreal frameMultiplier = 1,
                          const qreal valueMultiplier = 1) const;

    void setPrefferedValueStep(const qreal valueStep);

    void setValueRange(const qreal minVal, const qreal maxVal);
    void setMinValue(const qreal minVal) {
        mMinPossibleVal = minVal;
        setCurrentBaseValue(mCurrentBaseValue);
    }
    void setMaxValue(const qreal maxVal) {
        mMaxPossibleVal = maxVal;
        setCurrentBaseValue(mCurrentBaseValue);
    }

    void setCurrentBaseValue(qreal newValue);
    void setCurrentBaseValueNoUpdate(const qreal newValue);
    void incCurrentBaseValue(const qreal incBy);
    void incCurrentValueNoUpdate(const qreal incBy);
    void multCurrentBaseValue(const qreal mult);

    qreal getCurrentBaseValue() const;
    qreal getCurrentEffectiveValue() const;
    qreal getBaseValue(const qreal relFrame) const;
    qreal getBaseValueAtAbsFrame(const qreal frame) const;
    qreal getEffectiveValue(const qreal relFrame) const;
    qreal getEffectiveValueAtAbsFrame(const qreal frame) const;

    qreal getSavedBaseValue();
    void incAllValues(const qreal valInc);

    qreal getMinPossibleValue();
    qreal getMaxPossibleValue();

    qreal getPrefferedValueStep();

    void graphFixMinMaxValues();

    void saveValueToKey(const int frame, const qreal value);
    void removeThisFromGraphAnimator();

    void incSavedValueToCurrentValue(const qreal incBy);
    void multSavedValueToCurrentValue(const qreal multBy);

    int getNumberDecimals() { return mDecimals; }
    void setNumberDecimals(const int decimals) { mDecimals = decimals; }

    bool getBeingTransformed() { return mTransformed; }

    void setGenerator(const qsptr<RandomQrealGenerator> &generator);

    bool hasNoise();

    static auto sCreate0to1Animator(const QString& name) {
        auto anim = enve::make_shared<QrealAnimator>(0, 0, 1, 0.01, name);
        anim->graphFixMinMaxValues();
        return anim;
    }
private:
    qreal calculateBaseValueAtRelFrame(const qreal frame) const;

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

class QrealAction {
    enum Type { START, SET, FINISH };
    QrealAction(const qreal value, const Type type) :
        mValue(value), mType(type) {}
public:
    void apply(QrealAnimator* const target) const {
        if(mType == START) target->prp_startTransform();
        else if(mType == SET) target->setCurrentBaseValue(mValue);
        else if(mType == FINISH) target->prp_finishTransform();
    }

    static QrealAction sMakeStart()
    { return QrealAction{0., START}; }
    static QrealAction sMakeSet(const qreal value)
    { return QrealAction{value, SET}; }
    static QrealAction sMakeFinish()
    { return QrealAction{0., FINISH}; }
private:
    qreal mValue;
    Type mType;
};

#endif // VALUEANIMATORS_H
