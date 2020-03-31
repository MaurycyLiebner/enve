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

#ifndef SCULPTPATH_H
#define SCULPTPATH_H
#include "sculptnode.h"
#include "sculptbrush.h"
#include "smartPointers/stdselfref.h"
#include "skia/skiaincludes.h"
#include "Paint/brushstrokeset.h"

enum class SculptMode {
    drag, add, replace, subtract
};

enum class SculptTarget {
    position,
    width,
    pressure,
    spacing,
    time,
    color
};

class CORE_EXPORT SculptPath {
public:
    SculptPath();
    SculptPath(const SkPath& path, const qreal spacing);
    SculptPath(const SculptPath& other);

    SculptPath& operator=(const SculptPath& other);

    void setPath(const SkPath& path, const qreal spacing);

    BrushStrokeSet generateBrushSet(const QMatrix& transform,
                                    const qreal width) const;

    void sculpt(const SculptTarget target,
                const SculptMode mode,
                const SculptBrush &brush);

    int remesh(const qreal spacing);
    int remesh(const qreal t0, const qreal t1, const qreal spacing);
    int remesh(const IdRange &idRange, const qreal spacing);

    void applyTransform(const QMatrix &transform);

    void read(eReadStream& src);
    void write(eWriteStream &dst) const;

    QRectF boundingRect() const { return mBoundingRect; }

    static SculptPath sInterpolate(const SculptPath& path1,
                                   const SculptPath& path2,
                                   const qreal path1Infl);

    const QList<stdsptr<SculptNode>>& nodes() const
    { return mNodes; }
private:
    void updateBoundingRect();

    using NodeOp = bool (SculptNode::*)(const SculptBrush&);
    void allNodesOperation(NodeOp op, const SculptBrush& brush);
    void allNodesOperationRemesh(NodeOp op, const SculptBrush& brush);

    using NodeProperty = qreal (SculptNode::*)() const;
    qreal allNodesSample(NodeProperty op, const SculptBrush &brush);
    QColor allNodesColorSample(const SculptBrush &brush);

    void dragPosition(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::dragPosition, brush); }
    void replacePosition(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replacePosition, brush); }

    void addWidth(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::addWidth, brush); }
    void substractWidth(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::substractWidth, brush); }
    void replaceWidth(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replaceWidth, brush); }

    void addPressure(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::addPressure, brush); }
    void substractPressure(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::substractPressure, brush); }
    void replacePressure(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replacePressure, brush); }

    void addSpacing(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::addSpacing, brush); }
    void substractSpacing(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::substractSpacing, brush); }
    void replaceSpacing(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replaceSpacing, brush); }

    void addTime(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::addTime, brush); }
    void substractTime(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::substractTime, brush); }
    void replaceTime(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replaceTime, brush); }

    void addColor(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::addColor, brush); }
    void substractColor(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::substractColor, brush); }
    void replaceColor(const SculptBrush& brush)
    { allNodesOperation(&SculptNode::replaceColor, brush); }

    std::pair<SculptNode*, SculptNode*> nodesAroundT(const qreal t) const;
    SculptNode nodeAtT(const qreal t) const;
    IdRange tRangeToIdRange(const qValueRange& range) const;

    QList<stdsptr<SculptNode>> mNodes;
    QRectF mBoundingRect;
};

#endif // SCULPTPATH_H
