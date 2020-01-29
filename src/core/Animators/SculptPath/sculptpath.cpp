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

#include "sculptpath.h"
#include "Segments/cubiclist.h"
#include "ReadWrite/ereadstream.h"
#include "ReadWrite/ewritestream.h"
#include <limits>

SculptPath::SculptPath() {}

SculptPath::SculptPath(const SkPath& path, const qreal spacing) {
    setPath(path, spacing);
}

SculptPath::SculptPath(const SculptPath &other) {
    *this = other;
}

SculptPath &SculptPath::operator=(const SculptPath &other) {
    mNodes.clear();
    for(const auto& otherNode : other.mNodes) {
        mNodes.append(std::make_shared<SculptNode>(*otherNode));
    }
    return *this;
}

void SculptPath::setPath(const SkPath &path, const qreal spacing) {
    mNodes.clear();
    if(path.isEmpty()) return;
    SkPathMeasure meas(path, false);
    const float length = meas.getLength();
    const int nTot = SkScalarCeilToInt(length / spacing);
    const float segLen = length / nTot;
    float dist = 0;
    for(int i = 0; i <= nTot; i++) {
        SkPoint pos;
        if(meas.getPosTan(dist += segLen, &pos, nullptr)) {
            const auto qPos = toQPointF(pos);
            const qreal t = 100.*i/nTot;
            const auto node = new SculptNode(t, qPos, 5, 1, 2, 0.1);
            mNodes.append(stdsptr<SculptNode>(node));
        }
    }
}

BrushStroke generateBrushStroke(const QMatrix &transform,
                                const qreal width,
                                SculptNode* const startNode,
                                SculptNode* const endNode) {

    BrushStroke result;

    result.fXTilt = DEFAULT_TILT_CURVE;
    result.fYTilt = DEFAULT_TILT_CURVE;

    const auto lastPos = transform.map(startNode->pos());
    const auto iPos = transform.map(endNode->pos());
    result.fStrokePath = qCubicSegment2D(lastPos, lastPos, iPos, iPos);

    const auto lastWidth = startNode->width()*width;
    const auto iWidth = endNode->width()*width;
    result.fWidthCurve = qCubicSegment1D::sMakeLinearToT(lastWidth, iWidth);

    const auto lastPress = startNode->pressure();
    const auto iPress = endNode->pressure();
    result.fPressure = qCubicSegment1D::sMakeLinearToT(lastPress, iPress);

    const auto lastSpacing = startNode->spacing();
    const auto iSpacing = endNode->spacing();
    result.fSpacingCurve = qCubicSegment1D::sMakeLinearToT(lastSpacing, iSpacing);

    const auto lastTime = 0.1*startNode->time();
    const auto iTime = 0.1*endNode->time();
    result.fTimeCurve = qCubicSegment1D::sMakeLinearToT(lastTime, iTime);

    result.fUseColor = true;
    result.fColor = startNode->color();

    return result;
}

BrushStrokeSet SculptPath::generateBrushSet(const QMatrix &transform,
                                            const qreal width) const {
    if(mNodes.count() < 2) return BrushStrokeSet();
    BrushStrokeSet result;

    SculptNode* prevNode = mNodes.first().get();
    for(int i = 1; i < mNodes.count(); i++) {
        SculptNode* const iNode = mNodes.at(i).get();
        result.fStrokes << generateBrushStroke(transform, width,
                                               prevNode, iNode);
        prevNode = iNode;
    }

    return result;
}

SculptNode SculptPath::nodeAtT(const qreal t) const {
    const auto between = nodesAroundT(t);
    if(!between.first && !between.second) {
        return SculptNode(t, QPointF(0, 0), 0, 0, 10, 1);
    }
    if(!between.first && between.second) return *between.second;
    if(between.first && !between.second) return *between.first;
    return SculptNode::sInterpolateT(*between.first, *between.second, t);
}

IdRange SculptPath::tRangeToIdRange(const qValueRange &range) const {
    const auto compPrev =
    [](const stdsptr<SculptNode>& node, const qreal t) {
        return node->t() < t;
    };
    const auto first = std::lower_bound(mNodes.begin(), mNodes.end(),
                                        range.fMin, compPrev);
    const int firstId = first - mNodes.begin();
 //
    const auto compNext =
    [](const qreal t, const stdsptr<SculptNode>& node) {
        return t < node->t();
    };
    const auto last = std::upper_bound(mNodes.begin(), mNodes.end(),
                                       range.fMax, compNext);
    const int lastId = last - mNodes.begin();

    return {qBound(0, firstId, mNodes.count() - 1),
            qBound(0, lastId,  mNodes.count() - 1)};
}

int SculptPath::remesh(const IdRange& idRange, const qreal spacing) {
    if(mNodes.count() < 2) return 0;
    QList<stdsptr<SculptNode>> replacer;

    const int iMin = qMax(0, idRange.fMin);
    const int iMax = qMin(mNodes.count() - 1, idRange.fMax);
    SculptNode* prevNode = mNodes.at(iMin).get();
    const qreal maxAcceptableDist = 1.1*spacing;
    // create new nodes
    for(int i = iMin + 1; i <= iMax; i++) {
        const auto iNode = mNodes.at(i).get();
        const qreal distFromPrev = pointToLen(prevNode->pos() - iNode->pos());
        if(distFromPrev < maxAcceptableDist) continue;
        const qreal iNodeInfl = spacing/distFromPrev;
        auto node = SculptNode::sInterpolateInfl(*iNode, *prevNode, iNodeInfl);
        const auto newNode = new SculptNode(node);
        replacer << stdsptr<SculptNode>(newNode);
        prevNode = newNode;
        i--;
    }

    const int oldCount = mNodes.count();

    // remove old nodes
    for(int i = iMax - 1; i >= iMin + 1; i--) {
        mNodes.removeAt(i);
    }

    // add new nodes
    const int insertId = qMax(0, iMin + 1);
    for(int i = replacer.count() - 1; i >= 0; i--) {
        mNodes.insert(insertId, replacer.at(i));
    }

    return mNodes.count() - oldCount;
}

int SculptPath::remesh(const qreal spacing) {
    return remesh({-1, mNodes.count()}, spacing);
}

int SculptPath::remesh(const qreal t0, const qreal t1, const qreal spacing) {
    if(mNodes.count() < 2) return 0;
    // node ids to replace
    const auto idRange = tRangeToIdRange({t0, t1});
    return remesh(idRange, spacing);
}

void SculptPath::applyTransform(const QMatrix &transform) {
    for(const auto& node : mNodes) {
        node->setPos(transform.map(node->pos()));
    }
}

void SculptPath::read(eReadStream &src) {
    mNodes.clear();
    int nNodes; src >> nNodes;
    for(int i = 0; i < nNodes; i++) {
        const auto node = new SculptNode;
        src.read(node, sizeof(SculptNode));
        mNodes << stdsptr<SculptNode>(node);
    }
    src >> mBoundingRect;
}

void SculptPath::write(eWriteStream &dst) const {
    dst << mNodes.count();
    for(const auto& node : mNodes) {
        dst.write(node.get(), sizeof(SculptNode));
    }
    dst << mBoundingRect;
}

void SculptPath::sculpt(const SculptTarget target,
                        const SculptMode mode,
                        const SculptBrush &brush) {
    switch(target) {
    case SculptTarget::position: {
        switch(mode) {
        case SculptMode::drag: {
            allNodesOperationRemesh(&SculptNode::dragPosition, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperationRemesh(&SculptNode::replacePosition, brush);
        } break;
        default: break;
        }
        updateBoundingRect();
    } break;
    case SculptTarget::width: {
        switch(mode) {
        case SculptMode::drag: {
            brush.setTmpValue(allNodesSample(&SculptNode::width, brush));
            allNodesOperation(&SculptNode::dragWidth, brush);
        } break;
        case SculptMode::add: {
            allNodesOperation(&SculptNode::addWidth, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperation(&SculptNode::replaceWidth, brush);
        } break;
        case SculptMode::subtract: {
            allNodesOperation(&SculptNode::substractWidth, brush);
        } break;
        }
    } break;
    case SculptTarget::pressure: {
        switch(mode) {
        case SculptMode::drag: {
            brush.setTmpValue(allNodesSample(&SculptNode::pressure, brush));
            allNodesOperation(&SculptNode::dragPressure, brush);
        } break;
        case SculptMode::add: {
            allNodesOperation(&SculptNode::addPressure, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperation(&SculptNode::replacePressure, brush);
        } break;
        case SculptMode::subtract: {
            allNodesOperation(&SculptNode::substractPressure, brush);
        } break;
        }
    } break;
    case SculptTarget::spacing: {
        switch(mode) {
        case SculptMode::drag: {
            brush.setTmpValue(allNodesSample(&SculptNode::spacing, brush));
            allNodesOperation(&SculptNode::dragSpacing, brush);
        } break;
        case SculptMode::add: {
            allNodesOperation(&SculptNode::addSpacing, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperation(&SculptNode::replaceSpacing, brush);
        } break;
        case SculptMode::subtract: {
            allNodesOperation(&SculptNode::substractSpacing, brush);
        } break;
        }
    } break;
    case SculptTarget::time: {
        switch(mode) {
        case SculptMode::drag: {
            brush.setTmpValue(allNodesSample(&SculptNode::time, brush));
            allNodesOperation(&SculptNode::dragTime, brush);
        } break;
        case SculptMode::add: {
            allNodesOperation(&SculptNode::addTime, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperation(&SculptNode::replaceTime, brush);
        } break;
        case SculptMode::subtract: {
            allNodesOperation(&SculptNode::substractTime, brush);
        } break;
        }
    } break;
    case SculptTarget::color: {
        switch(mode) {
        case SculptMode::drag: {
            brush.setTmpColor(allNodesColorSample(brush));
            allNodesOperation(&SculptNode::dragColor, brush);
        } break;
        case SculptMode::add: {
            allNodesOperation(&SculptNode::addColor, brush);
        } break;
        case SculptMode::replace: {
            allNodesOperation(&SculptNode::replaceColor, brush);
        } break;
        case SculptMode::subtract: {
            allNodesOperation(&SculptNode::substractColor, brush);
        } break;
        }
    } break;
    }
}

SculptPath SculptPath::sInterpolate(const SculptPath &path1,
                                    const SculptPath &path2,
                                    const qreal path1Infl) {
    SculptPath result;
    if(path1.mNodes.count() < 2 ||
       path2.mNodes.count() < 2) return result;
    auto it1 = path1.mNodes.begin();
    auto it2 = path2.mNodes.begin();
    const auto end1 = path1.mNodes.end();
    const auto end2 = path2.mNodes.end();

    while(it1 != end1 && it2 != end2) {
        const auto& n1 = **it1;
        const auto& n2 = **it2;
        if(isZero6Dec(n1.t() - n2.t())) {
            it1++;
            it2++;
            const auto node = std::make_shared<SculptNode>(
                        SculptNode::sInterpolateInfl(n1, n2, path1Infl));
            result.mNodes << node;
        } else if(n1.t() < n2.t()) {
            const qreal t = n1.t();
            it1++;
            const auto n2i = path2.nodeAtT(t);
            const auto node = std::make_shared<SculptNode>(
                        SculptNode::sInterpolateInfl(n1, n2i, path1Infl));
            result.mNodes << node;
        } else {
            const qreal t = n2.t();
            it2++;
            const auto n1i = path1.nodeAtT(t);
            const auto node = std::make_shared<SculptNode>(
                        SculptNode::sInterpolateInfl(n1i, n2, path1Infl));
            result.mNodes << node;
        }
    }
    result.remesh(5);
    return result;
}

void SculptPath::updateBoundingRect() {
    if(mNodes.isEmpty()) {
        mBoundingRect = QRect(0, 0, 0, 0);
        return;
    }
    qreal minLeft = DBL_MAX;
    qreal minTop = DBL_MAX;
    qreal maxRight = -DBL_MAX;
    qreal maxBottom = -DBL_MAX;
    for(const auto& node : mNodes) {
        const auto pos = node->pos();
        minLeft = qMin(minLeft, pos.x());
        minTop = qMin(minTop, pos.y());
        maxRight = qMax(maxRight, pos.x());
        maxBottom = qMax(maxBottom, pos.y());
    }
    const QPoint tl(qFloor(minLeft), qFloor(minTop));
    const QPoint br(qCeil(maxRight), qCeil(maxBottom));
    mBoundingRect = QRect(tl, br);
}

qreal SculptPath::allNodesSample(NodeProperty op, const SculptBrush &brush) {
    qreal influenceSum = 0;
    qreal valueSum = 0;
    for(const auto& node : mNodes) {
        const qreal nInfluence = brush.influence(node->pos());
        if(isZero4Dec(nInfluence)) continue;
        const qreal nodeVal = ((*node).*op)();
        valueSum += nodeVal*nInfluence;
        influenceSum += nInfluence;
    }

    if(isZero4Dec(influenceSum)) return 0;
    else return valueSum/influenceSum;
}

QColor SculptPath::allNodesColorSample(const SculptBrush &brush) {
    const qreal red = allNodesSample(&SculptNode::redF, brush);
    const qreal green = allNodesSample(&SculptNode::greenF, brush);
    const qreal blue = allNodesSample(&SculptNode::blueF, brush);
    const qreal alpha = allNodesSample(&SculptNode::alphaF, brush);
    return QColor::fromRgbF(red, green, blue, alpha);
}

void SculptPath::allNodesOperation(NodeOp op, const SculptBrush &brush) {
    for(const auto& node : mNodes) ((*node).*op)(brush);
}

void SculptPath::allNodesOperationRemesh(SculptPath::NodeOp op,
                                         const SculptBrush &brush) {
    int rangeMin = 0;
    bool lastInfluenced = false;
    for(int i = 0; i < mNodes.count(); i++) {
        const auto& node = mNodes.at(i);
        const bool influenced = ((*node).*op)(brush);
        if(lastInfluenced && !influenced) {
            i += remesh(IdRange{rangeMin, i}, brush.nodeSpacing());
        } else if(!lastInfluenced && influenced) rangeMin = i - 1;
        lastInfluenced = influenced;
    }
    if(lastInfluenced) {
        remesh(IdRange{rangeMin, mNodes.count()}, brush.nodeSpacing());
    }
}

std::pair<SculptNode*, SculptNode*> SculptPath::nodesAroundT(const qreal t) const {
    const auto compPrev = [](const stdsptr<SculptNode>& node,
                             const qreal t) {
        return node->t() < t;
    };
    const auto prevIt = std::lower_bound(mNodes.begin(), mNodes.end(),
                                         t, compPrev);
    const int prevId = prevIt - mNodes.begin();
    const int nextId = prevId + 1;
    const auto prev = prevIt == mNodes.end() ?
                nullptr : prevIt->get();
    const auto next = nextId >= mNodes.count() ?
                nullptr : mNodes.at(nextId).get();
    return {prev, next};
}
