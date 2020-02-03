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

#ifndef EXPRESSIONSINGLECHILD_H
#define EXPRESSIONSINGLECHILD_H

#include "expressionvalue.h"

class ExpressionSingleChild : public ExpressionValue {
public:
    ExpressionSingleChild(const QString& name, const sptr& value);

    void collapse() override;
    bool isPlainValue() const override;

    bool setRelFrame(const qreal relFrame) override;

    bool isValid() const override
    { return mValue->isValid(); }
    FrameRange identicalRange(const qreal relFrame) const override
    { return mValue->identicalRange(relFrame); }
    bool dependsOn(QrealAnimator* const source) const override
    { return mValue->dependsOn(source); }
protected:
    qreal innerValue(const qreal relFrame) const
    { return mValue->value(relFrame); }
    QString innerString() const { return mValue->toString(); }
    const QString& name() const { return mName; }
private:
    const QString mName;
    sptr mValue;
};

#endif // EXPRESSIONSINGLECHILD_H
