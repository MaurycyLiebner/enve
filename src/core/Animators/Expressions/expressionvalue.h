#ifndef EXPRESSIONVALUE_H
#define EXPRESSIONVALUE_H
#include <QObject>
#include <QSharedPointer>

#include "framerange.h"
#include "simplemath.h"

class ExpressionValue : public QObject {
    Q_OBJECT
public:
    ExpressionValue() {}

    virtual qreal calculateValue(const qreal relFrame) const = 0;
    virtual bool isPlainValue() const = 0;
    virtual bool isValid() const = 0;
    virtual void collapse() = 0;
    virtual FrameRange identicalRange(const qreal relFrame) const = 0;

    virtual bool setRelFrame(const qreal relFrame) {
        setRelFrameValue(relFrame);
        return updateValue();
    }

    using sptr = QSharedPointer<ExpressionValue>;
public:
    qreal value(const qreal relFrame) const {
        if(!isValid()) return 1;
        const auto idRange = identicalRange(mRelFrame);
        const bool inRange = idRange.inRange(relFrame);
        if(inRange) return mCurrentValue;
        return calculateValue(relFrame);
    }

    qreal currentValue() const { return mCurrentValue; }
    bool updateValue() {
        const qreal newValue = calculateValue(mRelFrame);
        if(isZero4Dec(mCurrentValue - newValue)) return false;
        mCurrentValue = newValue;
        emit currentValueChanged(mCurrentValue);
        return true;
    }

    void setRelFrameValue(const qreal relFrame) {
        mRelFrame = relFrame;
    }
signals:
    void relRangeChanged(const FrameRange& range);
    void currentValueChanged(const qreal value);
private:
    qreal mRelFrame;
    qreal mCurrentValue;
};

#endif // EXPRESSIONVALUE_H
