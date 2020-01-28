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

#include "sculptnode.h"
#include "simplemath.h"

const qValueRange SculptNode::sWidthRange{0, 100};
const qValueRange SculptNode::sPressureRange{0, 1};
const qValueRange SculptNode::sSpacingRange{0, 10};
const qValueRange SculptNode::sTimeRange{0, 1};

SculptNode::SculptNode(const qreal t, const QPointF &pos,
                       const qreal width, const qreal pressure,
                       const qreal spacing, const qreal time,
                       const QColor &color) :
    SculptNodeBase(t, pos),
    mWidth(width), mPressure(pressure),
    mSpacing(spacing), mTime(time),
    mColor(color) {}

bool SculptNode::dragColor(const SculptBrush &brush) {
    return colorOp(&SculptNode::replaceProperty, brush, brush.tmpColor());
}

bool SculptNode::addColor(const SculptBrush &brush) {
    return colorOp(&SculptNode::addProperty, brush);
}

bool SculptNode::substractColor(const SculptBrush &brush) {
    return colorOp(&SculptNode::substractProperty, brush);
}

bool SculptNode::replaceColor(const SculptBrush &brush) {
    return colorOp(&SculptNode::replaceProperty, brush);
}

SculptNode SculptNode::sInterpolateT(const SculptNode &node1,
                                     const SculptNode &node2,
                                     const qreal t) {
    if(t < node1.t()) return node1;
    if(t > node2.t()) return node2;
    const qreal t1 = node1.t();
    const qreal t2 = node2.t();
    const qreal node1Infl = (t - t1)/(t2 - t1);
    return sInterpolateInfl(node1, node2, node1Infl);
}

SculptNode SculptNode::sInterpolateInfl(const SculptNode &node1,
                                        const SculptNode &node2,
                                        const qreal node1Infl) {
    const qreal node2Infl = 1 - node1Infl;
    const qreal t = node1.t()*node1Infl +
                    node2.t()*node2Infl;
    const auto pos = node1.pos()*node1Infl +
                     node2.pos()*node2Infl;
    const qreal width = node1.width()*node1Infl +
                        node2.width()*node2Infl;
    const qreal pressure = node1.pressure()*node1Infl +
                           node2.pressure()*node2Infl;
    const qreal spacing = node1.spacing()*node1Infl +
                          node2.spacing()*node2Infl;
    const qreal time = node1.time()*node1Infl +
                       node2.time()*node2Infl;
    const QColor& color1 = node1.color();
    const QColor& color2 = node2.color();
    const qreal red = color1.redF()*node1Infl +
                      color2.redF()*node2Infl;
    const qreal green = color1.greenF()*node1Infl +
                        color2.greenF()*node2Infl;
    const qreal blue = color1.blueF()*node1Infl +
                       color2.blueF()*node2Infl;
    const qreal alpha = color1.alphaF()*node1Infl +
                        color2.alphaF()*node2Infl;
    return SculptNode(t, pos, width, pressure, spacing, time,
                      QColor::fromRgbF(red, green, blue, alpha));
}

bool SculptNode::dragProperty(qreal &value, const qValueRange &range,
                              const SculptBrush &brush) {
    return replaceProperty(value, range, brush, brush.tmpValue());
}

bool SculptNode::addProperty(qreal& value,
                             const qValueRange& range,
                             const SculptBrush &brush,
                             const qreal brushValue) {
    const qreal influence = brush.influence(pos());
    return addProperty(value, range, brushValue, influence);
}

bool SculptNode::substractProperty(qreal& value,
                                   const qValueRange &range,
                                   const SculptBrush& brush,
                                   const qreal brushValue) {
    const qreal influence = brush.influence(pos());
    return substractProperty(value, range, brushValue, influence);
}

bool SculptNode::replaceProperty(qreal& value,
                                 const qValueRange &range,
                                 const SculptBrush& brush,
                                 const qreal brushValue) {
    const qreal influence = brush.influence(pos());
    return replaceProperty(value, range, brushValue, influence);
}

bool SculptNode::addProperty(qreal& value,
                             const qValueRange& range,
                             const qreal brushValue,
                             const qreal influence) {
    if(isZero4Dec(influence)) return false;
    const qreal unclamped = value + brushValue*influence;
    value = range.clamp(unclamped);
    return true;
}

bool SculptNode::substractProperty(qreal& value,
                                   const qValueRange &range,
                                   const qreal brushValue,
                                   const qreal influence) {
  if(isZero4Dec(influence)) return false;
  const qreal unclamped = value - brushValue*influence;
  value = range.clamp(unclamped);
  return true;
}

bool SculptNode::replaceProperty(qreal& value,
                                 const qValueRange &range,
                                 const qreal brushValue,
                                 const qreal influence) {
    if(isZero4Dec(influence)) return false;
    const qreal unclamped = value*(1 - influence) + brushValue*influence;
    value = range.clamp(unclamped);
    return true;
}

bool SculptNode::colorOp(const ColorOp &op, const SculptBrush &brush) {
    return colorOp(op, brush, brush.color());
}

bool SculptNode::colorOp(const ColorOp &op, const SculptBrush &brush,
                         const QColor& brushColor) {
    const qreal influence = brush.influence(pos());
    if(isZero4Dec(influence)) return false;
    qreal redF = mColor.redF();
    qreal greenF = mColor.greenF();
    qreal blueF = mColor.blueF();
    qreal alphaF = mColor.alphaF();
    (this->*op)(redF, {0, 1}, brushColor.redF(), influence);
    (this->*op)(greenF, {0, 1}, brushColor.greenF(), influence);
    (this->*op)(blueF, {0, 1}, brushColor.blueF(), influence);
    (this->*op)(alphaF, {0, 1}, brushColor.alphaF(), influence);
    mColor = QColor::fromRgbF(redF, greenF, blueF, alphaF);
    return true;
}
