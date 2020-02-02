// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    int relFrame() const { return mRelFrame; }

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
