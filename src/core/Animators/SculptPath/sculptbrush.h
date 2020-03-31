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

#ifndef SCULPTBRUSH_H
#define SCULPTBRUSH_H

#include "../../core_global.h"

#include <QPointF>
#include <QMatrix>
#include <QColor>

class CORE_EXPORT SculptBrush {
public:
    SculptBrush(const qreal expRadius,
                const qreal hardness,
                const qreal opacity,
                const qreal value = 1);
    SculptBrush(const QMatrix& transform,
                const SculptBrush& other);

    QPointF center() const { return mPos2; }
    qreal radius() const { return mRadius; }
    void incExpRadius(const qreal inc);
    void decExpRadius(const qreal dec)
    { return incExpRadius(-dec); }

    qreal hardness() const { return mHardness; }
    void setHardness(const qreal hardness);

    qreal opacity() const { return mOpacity; }
    void setOpacity(const qreal opacity);

    qreal value() const { return mValue; }
    void setValue(const qreal value);

    qreal redF() const { return mColor.redF(); }
    qreal greenF() const { return mColor.greenF(); }
    qreal blueF() const { return mColor.blueF(); }
    qreal alphaF() const { return mColor.alphaF(); }

    const QColor& color() const { return mColor; }
    void setColor(const QColor& color);

    qreal tmpValue() const { return mTmpValue; }
    void setTmpValue(const qreal value) const { mTmpValue = value; }

    const QColor& tmpColor() const { return mTmpColor; }
    void setTmpColor(const QColor& color) const { mTmpColor = color; }

    QPointF closestOuterPoint(const QPointF& pos) const;
    QPointF displacement() const;
    qreal influence(const QPointF& pos) const;

    qreal nodeSpacing() const { return mNodeSpacing; }

    void pressAt(const QPointF& pos, const qreal pressure);
    void moveTo(const QPointF& pos, const qreal pressure);
private:
    void updateRadius();
    void updateSlopeAndOffset();
    void map(const QMatrix& transform);

    QPointF mPos1; // previous position
    QPointF mPos2; // current position
    qreal mPressure;

    qreal mNodeSpacing = 5;

    qreal mExpRadius;

    qreal mRadius;
    qreal mHardness;
    qreal mOpacity;

    qreal mValue;
    mutable qreal mTmpValue;

    QColor mColor;
    mutable QColor mTmpColor;

    qreal mSeg1Slope;
    qreal mSeg1Offset;
    qreal mSeg2Offset;
    qreal mSeg2Slope;
};

#endif // SCULPTBRUSH_H
