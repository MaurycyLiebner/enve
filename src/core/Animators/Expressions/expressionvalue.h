#ifndef EXPRESSIONVALUE_H
#define EXPRESSIONVALUE_H
#include <QObject>
#include <QSharedPointer>

#include "framerange.h"
#include "simplemath.h"

class ExpressionValue : public QObject {
    Q_OBJECT
public:
    ExpressionValue(const bool needsBrackets);

    virtual qreal calculateValue(const qreal relFrame) const = 0;
    virtual bool isPlainValue() const = 0;
    virtual bool isValid() const = 0;
    virtual void collapse() = 0;
    virtual FrameRange identicalRange(const qreal relFrame) const = 0;
    virtual QString toString() const = 0;

    virtual bool setRelFrame(const qreal relFrame);

    using sptr = QSharedPointer<ExpressionValue>;
public:
    qreal currentValue() const { return mCurrentValue; }
    qreal value(const qreal relFrame) const;
    bool updateValue();

    bool needsBrackets() const
    { return mNeedsBrackets; }
protected:
    void setRelFrameValue(const qreal relFrame)
    { mRelFrame = relFrame; }
signals:
    void relRangeChanged(const FrameRange& range);
    void currentValueChanged(const qreal value);
private:
    const bool mNeedsBrackets;
    qreal mRelFrame;
    qreal mCurrentValue;
};

#endif // EXPRESSIONVALUE_H
