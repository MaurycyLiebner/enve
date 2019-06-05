#ifndef GRAPHKEY_H
#define GRAPHKEY_H
#include "key.h"
enum QrealPointType : short;
class QrealPoint;

class ClampedValue {
    qreal mValue;
    qreal mSavedValue;
    qreal mMin = -DBL_MAX;
    qreal mMax = DBL_MAX;
public:
    ClampedValue(const qreal& value) : mValue(value) {}

    void saveValue() {
        mSavedValue = mValue;
    }

    void restoreSavedValue() {
        mValue = mSavedValue;
    }

    qreal getRawSavedValue() const {
        return mSavedValue;
    }

    qreal getRawValue() const {
        return mValue;
    }

    qreal getClampedSavedValue() const {
        return clamp(mSavedValue, mMin, mMax);
    }

    qreal getClampedValue() const {
        return clamp(mValue, mMin, mMax);
    }

    void setValue(const qreal& value) {
        mValue = value;
    }

    void clampValue() {
        mValue = getClampedValue();
    }

    void setRange(const qreal& min, const qreal& max) {
        mMin = min;
        mMax = max;
    }

    void setMax(const qreal& max) {
        mMax = max;
    }

    void setMin(const qreal& min) {
        mMin = min;
    }
};

#include "basicreadwrite.h"
class ClampedPoint {
    ClampedValue mX;
    ClampedValue mY;
public:
    ClampedPoint(const qreal& x, const qreal& y) : mX(x), mY(y) {}

    void read(QIODevice * const target) {
        target->read(rcChar(&mX), sizeof(qreal));
        target->read(rcChar(&mY), sizeof(qreal));
    }

    void saveXValue() {
        mX.saveValue();
    }

    void restoreSavedXValue() {
        mX.restoreSavedValue();
    }

    void saveYValue() {
        mY.saveValue();
    }

    void restoreSavedYValue() {
        mY.restoreSavedValue();
    }

    void saveValue() {
        mX.saveValue();
        mY.saveValue();
    }

    void restoreSavedValue() {
        mX.restoreSavedValue();
        mY.restoreSavedValue();
    }

    QPointF getRawSavedValue() const {
        return {mX.getRawSavedValue(), mY.getRawSavedValue()};
    }

    QPointF getRawValue() const {
        return {mX.getRawValue(), mY.getRawValue()};
    }

    qreal getRawSavedXValue() const {
        return mX.getRawSavedValue();
    }

    qreal getRawXValue() const {
        return mX.getRawValue();
    }

    qreal getRawSavedYValue() const {
        return mY.getRawSavedValue();
    }

    qreal getRawYValue() const {
        return mY.getRawValue();
    }

    QPointF getClampedSavedValue(const QPointF& relTo) const {
        const qreal xFracDiv = mX.getRawSavedValue() - relTo.x();
        qreal xFrac;
        if(isZero6Dec(xFracDiv)) xFrac = 1;
        else xFrac = (mX.getClampedSavedValue() - relTo.x())/xFracDiv;

        qreal yFracDiv = mY.getRawSavedValue() - relTo.y();
        qreal yFrac;
        if(isZero6Dec(yFracDiv)) yFrac = 1;
        else yFrac = (mY.getClampedSavedValue() - relTo.y())/yFracDiv;

        const qreal minFrac = qMin(xFrac, yFrac);
        const QPointF rawVals{mX.getRawSavedValue(), mY.getRawSavedValue()};
        return relTo + minFrac*(rawVals - relTo);
    }

    QPointF getClampedValue(const QPointF& relTo) const {
        const qreal xFracDiv = mX.getRawValue() - relTo.x();
        qreal xFrac;
        if(isZero6Dec(xFracDiv)) xFrac = 1;
        else xFrac = (mX.getClampedValue() - relTo.x())/xFracDiv;

        qreal yFracDiv = mY.getRawValue() - relTo.y();
        qreal yFrac;
        if(isZero6Dec(yFracDiv)) yFrac = 1;
        else yFrac = (mY.getClampedValue() - relTo.y())/yFracDiv;

        const qreal minFrac = qMin(xFrac, yFrac);
        const QPointF rawVals{mX.getRawValue(), mY.getRawValue()};
        return relTo + minFrac*(rawVals - relTo);
    }

    void setXValue(const qreal& value) {
        mX.setValue(value);
    }

    void setYValue(const qreal& value) {
        mY.setValue(value);
    }

    void setValue(const QPointF& value) {
        mX.setValue(value.x());
        mY.setValue(value.y());
    }

    void clampValue() {
        mX.clampValue();
        mY.clampValue();
    }

    void setXRange(const qreal& min, const qreal& max) {
        mX.setRange(min, max);
    }

    void setYRange(const qreal& min, const qreal& max) {
        mY.setRange(min, max);
    }

    void setXMax(const qreal& max) {
        mX.setMax(max);
    }

    void setXMin(const qreal& min) {
        mX.setMin(min);
    }

    void setYMax(const qreal& max) {
        mY.setMax(max);
    }

    void setYMin(const qreal& min) {
        mY.setMin(min);
    }
};

class GraphKey : public Key {
public:
    GraphKey(const int &frame,
             Animator * const parentAnimator);
    GraphKey(Animator * const parentAnimator);

    virtual void setValueForGraph(const qreal& value) = 0;
    virtual qreal getValueForGraph() const = 0;

    void startFrameAndValueTransform() {
        startFrameTransform();
        startValueTransform();
    }

    void finishFrameAndValueTransform() {
        finishFrameTransform();
        finishValueTransform();
    }

    void cancelFrameAndValueTransform() {
        cancelFrameTransform();
        cancelValueTransform();
    }

    virtual void startValueTransform() {}
    virtual void cancelValueTransform() {}
    virtual void finishValueTransform() {}

    void startFrameTransform() {
        Key::startFrameTransform();
        mStartPt.saveValue();
        mEndPt.saveValue();

        mSavedStartEnabled = mStartEnabled;
        mSavedEndEnabled = mEndEnabled;
    }

    void finishFrameTransform() {
    //    if(!mParentAnimator) return;
    //    mParentAnimator->addUndoRedo(
    //                new ChangeKeyFrameUndoRedo(mSavedRelFrame,
    //                                           mRelFrame, this));
    }

    void cancelFrameTransform() {
        Key::cancelFrameTransform();
        setStartFrameVar(mStartPt.getRawSavedXValue());
        setEndFrameVar(mEndPt.getRawSavedXValue());
        setStartValueVar(mStartPt.getRawSavedYValue());
        setEndValueVar(mEndPt.getRawSavedYValue());
        setStartEnabledForGraph(mSavedStartEnabled);
        setEndEnabledForGraph(mSavedEndEnabled);
    }

    void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool& useSavedFrame);
protected:
    void setRelFrame(const int &frame);
public:
    qreal getEndValue() const;
    qreal getStartValue() const;

    void setEndValueForGraph(const qreal& value);
    void setStartValueForGraph(const qreal& value);

    void drawGraphKey(QPainter * const p,
                      const QColor &paintColor) const;

    qreal getEndValueDirectionForGraph() const {
        return getEndValueDirectionForGraphForEndValue(
                    getEndValue());
    }

    qreal getStartValueDirectionForGraph() const {
        return getStartValueDirectionForGraphForStartValue(
                    getStartValue());
    }

    void setEndValueDirectionForGraph(const qreal &value) {
        if(!hasNextKey()) return;
        setEndValueForGraph(
                    getEndValueForGraphEndValueDirection(value));
    }

    void setStartValueDirectionForGraph(const qreal &value) {
        if(!hasPrevKey()) return;
        setStartValueForGraph(
                    getStartValueForGraphStartValueDirection(value));
    }

    void setStartFrame(const qreal& frame);
    void setEndFrame(const qreal& frame);

    QrealPoint *mousePress(const qreal &frameT,
                           const qreal &valueT,
                           const qreal &pixelsPerFrame,
                           const qreal &pixelsPerValue);

    void updateCtrlFromCtrl(const QrealPointType &type);
    void setCtrlsMode(const CtrlsMode &mode);
    const CtrlsMode& getCtrlsMode() const;

    void constrainStartCtrlMinFrame(const qreal &minRelFrame);
    void constrainEndCtrlMaxFrame(const qreal &maxRelFrame);
    virtual void changeFrameAndValueBy(const QPointF &frameValueChange);

    qreal getPrevKeyValueForGraph() const {
        auto prevKey = getPrevKey<GraphKey>();
        if(!prevKey) return getValueForGraph();
        return prevKey->getValueForGraph();
    }
    qreal getNextKeyValueForGraph() const {
        auto nextKey = getNextKey<GraphKey>();
        if(!nextKey) return getValueForGraph();
        return nextKey->getValueForGraph();
    }

    void makeStartAndEndSmooth();
    bool isInsideRect(const QRectF &valueFrameRect) const;

    void setStartFrameVar(const qreal &startFrame);
    void setEndFrameVar(const qreal &endFrame);

    qreal getStartFrame() const;
    qreal getEndFrame() const;

    qreal getStartAbsFrame() const;
    qreal getEndAbsFrame() const;

    void setStartEnabledForGraph(const bool &bT);
    void setEndEnabledForGraph(const bool &bT);

    bool getEndEnabledForGraph() const;
    bool getStartEnabledForGraph() const;

    void setStartValueVar(const qreal &value);
    void setEndValueVar(const qreal &value);
    void constrainEndCtrlValue(const qreal &minVal,
                               const qreal &maxVal);
    void constrainStartCtrlValue(const qreal &minVal,
                                 const qreal &maxVal);
protected:
    qreal getEndValueDirectionForGraphForEndValue(const qreal& endVal) const {
        if(!hasNextKey()) return 0;
        qreal nextValue = getNextKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = getNextKeyRelFrame() - mRelFrame;
        return (endVal - valG)/(nextValue - valG)/dFrame;
    }

    qreal getStartValueDirectionForGraphForStartValue(const qreal& startVal) const {
        if(!hasPrevKey()) return 0;
        qreal prevValue = getPrevKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = mRelFrame - getPrevKeyRelFrame();
        return (startVal - valG)/(prevValue - valG)/dFrame;
    }

    qreal getStartValueForGraphStartValueDirection(const qreal& value) const {
        if(!hasPrevKey()) return 0;
        qreal prevValue = getPrevKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = mRelFrame - getPrevKeyRelFrame();
        return valG + (prevValue - valG)*value*dFrame;
    }

    qreal getEndValueForGraphEndValueDirection(const qreal& value) const {
        if(!hasNextKey()) return 0;
        qreal nextValue = getNextKeyValueForGraph();
        qreal valG = getValueForGraph();
        int dFrame = getNextKeyRelFrame() - mRelFrame;
        return valG + (nextValue - valG)*value*dFrame;
    }

    bool mSavedStartEnabled = false;
    bool mSavedEndEnabled = false;

    bool mStartEnabled = false;
    bool mEndEnabled = false;

    ClampedPoint mStartPt = ClampedPoint(0, 0);
    ClampedPoint mEndPt = ClampedPoint(0, 0);

    stdsptr<QrealPoint> mGraphPoint;
    stdsptr<QrealPoint> mStartPoint;
    stdsptr<QrealPoint> mEndPoint;

    CtrlsMode mCtrlsMode = CTRLS_SYMMETRIC;
};

#endif // GRAPHKEY_H
