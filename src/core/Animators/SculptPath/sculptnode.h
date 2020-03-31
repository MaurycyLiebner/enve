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

#ifndef SCULPTNODE_H
#define SCULPTNODE_H

#include <QColor>

#include "sculptnodebase.h"
#include "framerange.h"
#include "sculptbrush.h"

class CORE_EXPORT SculptNode : public SculptNodeBase {
public:
    SculptNode() {}
    SculptNode(const qreal t, const QPointF& pos,
               const qreal width, const qreal pressure,
               const qreal spacing, const qreal time,
               const QColor& color = Qt::black);

    qreal width() const { return mWidth; }
    qreal pressure() const { return mPressure; }
    qreal spacing() const { return mSpacing; }
    qreal time() const { return mTime; }
    const QColor& color() const { return mColor; }

    qreal redF() const { return mColor.redF(); }
    qreal greenF() const { return mColor.greenF(); }
    qreal blueF() const { return mColor.blueF(); }
    qreal alphaF() const { return mColor.alphaF(); }

    bool dragWidth(const SculptBrush& brush)
    { return dragProperty(mWidth, sWidthRange, brush); }
    bool addWidth(const SculptBrush& brush)
    { return addProperty(mWidth, sWidthRange, brush); }
    bool substractWidth(const SculptBrush& brush)
    { return substractProperty(mWidth, sWidthRange, brush); }
    bool replaceWidth(const SculptBrush& brush)
    { return replaceProperty(mWidth, sWidthRange, brush); }

    bool dragPressure(const SculptBrush& brush)
    { return dragProperty(mPressure, sPressureRange, brush); }
    bool addPressure(const SculptBrush& brush)
    { return addProperty(mPressure, sPressureRange, brush); }
    bool substractPressure(const SculptBrush& brush)
    { return substractProperty(mPressure, sPressureRange, brush); }
    bool replacePressure(const SculptBrush& brush)
    { return replaceProperty(mPressure, sPressureRange, brush); }

    bool dragSpacing(const SculptBrush& brush)
    { return dragProperty(mSpacing, sSpacingRange, brush); }
    bool addSpacing(const SculptBrush& brush)
    { return addProperty(mSpacing, sSpacingRange, brush); }
    bool substractSpacing(const SculptBrush& brush)
    { return substractProperty(mSpacing, sSpacingRange, brush); }
    bool replaceSpacing(const SculptBrush& brush)
    { return replaceProperty(mSpacing, sSpacingRange, brush); }

    bool dragTime(const SculptBrush& brush)
    { return dragProperty(mTime, sTimeRange, brush); }
    bool addTime(const SculptBrush& brush)
    { return addProperty(mTime, sTimeRange, brush); }
    bool substractTime(const SculptBrush& brush)
    { return substractProperty(mTime, sTimeRange, brush); }
    bool replaceTime(const SculptBrush& brush)
    { return replaceProperty(mTime, sTimeRange, brush); }

    bool dragColor(const SculptBrush& brush);
    bool addColor(const SculptBrush& brush);
    bool substractColor(const SculptBrush& brush);
    bool replaceColor(const SculptBrush& brush);

    //! @brief Interpolate using distance from t as basis for influence
    static SculptNode sInterpolateT(const SculptNode& node1,
                                    const SculptNode& node2,
                                    const qreal t);
    //! @brief Interpolate using influence
    //! @param wrap - true when both nodes are end nodes
    static SculptNode sInterpolateInfl(const SculptNode& node1,
                                       const SculptNode& node2,
                                       const qreal node1Infl);
private:
    bool dragProperty(qreal& value, const qValueRange &range,
                      const SculptBrush& brush);

    bool addProperty(qreal& value, const qValueRange &range,
                     const SculptBrush& brush)
    { return addProperty(value, range, brush, brush.value()); }

    bool substractProperty(qreal& value, const qValueRange &range,
                           const SculptBrush& brush)
    { return substractProperty(value, range, brush, brush.value()); }

    bool replaceProperty(qreal& value, const qValueRange &range,
                         const SculptBrush& brush)
    { return replaceProperty(value, range, brush, brush.value()); }

    bool addProperty(qreal& value,
                     const qValueRange &range,
                     const SculptBrush& brush,
                     const qreal brushValue);
    bool substractProperty(qreal& value,
                           const qValueRange &range,
                           const SculptBrush& brush,
                           const qreal brushValue);
    bool replaceProperty(qreal& value,
                         const qValueRange &range,
                         const SculptBrush& brush,
                         const qreal brushValue);

    bool addProperty(qreal& value,
                     const qValueRange& range,
                     const qreal brushValue,
                     const qreal influence);
    bool substractProperty(qreal& value,
                           const qValueRange &range,
                           const qreal brushValue,
                           const qreal influence);
    bool replaceProperty(qreal& value,
                         const qValueRange &range,
                         const qreal brushValue,
                         const qreal influence);

    using ColorOp = bool (SculptNode::*)(qreal& value,
                                         const qValueRange& range,
                                         const qreal brushValue,
                                         const qreal influence);
    bool colorOp(const ColorOp& op, const SculptBrush &brush);
    bool colorOp(const ColorOp &op, const SculptBrush &brush,
                 const QColor& brushColor);

    qreal mWidth;
    qreal mPressure;
    qreal mSpacing;
    qreal mTime;
    QColor mColor;

    static const qValueRange sWidthRange;
    static const qValueRange sPressureRange;
    static const qValueRange sSpacingRange;
    static const qValueRange sTimeRange;
};

#endif // SCULPTNODE_H
