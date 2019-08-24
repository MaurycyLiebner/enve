// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#ifndef QPOINTFANIMATOR_H
#define QPOINTFANIMATOR_H
#include "staticcomplexanimator.h"
#include "../skia/skiaincludes.h"

class QPointFAnimator : public StaticComplexAnimator {
    e_OBJECT
protected:
    QPointFAnimator(const QString& name);
public:
    bool SWT_isQPointFAnimator() const { return true; }

    void setBaseValue(const qreal valX, const qreal valY) {
        setBaseValue({valX, valY});
    }
    void setBaseValue(const QPointF &val);

    void setBaseValueWithoutCallingUpdater(const QPointF &val);
    void incBaseValuesWithoutCallingUpdater(const qreal x, const qreal y);
    void incBaseValues(const qreal x, const qreal y);
    void multCurrentValues(const qreal sx, const qreal sy);

    QPointF getSavedValue();
    qreal getSavedXValue();
    qreal getSavedYValue();

    void incAllBaseValues(const qreal x, const qreal y);

    QrealAnimator *getXAnimator();
    QrealAnimator *getYAnimator();

    void multSavedValueToCurrentValue(const qreal sx,
                                      const qreal sy);
    void incSavedValueToCurrentValue(const qreal incXBy,
                                     const qreal incYBy);

    void setValuesRange(const qreal minVal, const qreal maxVal);

    QPointF getBaseValue() const;
    QPointF getBaseValueAtAbsFrame(const qreal frame) const;
    QPointF getBaseValue(const qreal relFrame) const;

    void setPrefferedValueStep(const qreal valueStep);

    bool getBeingTransformed();

    QPointF getEffectiveValue() const;
    QPointF getEffectiveValueAtAbsFrame(const qreal frame) const;
    QPointF getEffectiveValue(const qreal relFrame) const;

    qreal getEffectiveXValue();
    qreal getEffectiveXValue(const qreal relFrame);

    qreal getEffectiveYValue();
    qreal getEffectiveYValue(const qreal relFrame);
protected:
    qsptr<QrealAnimator> mXAnimator;
    qsptr<QrealAnimator> mYAnimator;
};

#endif // QPOINTFANIMATOR_H
