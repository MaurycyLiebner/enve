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

#include "graphanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {
    connect(this, &Animator::anim_addedKey, [this](Key * key) {
        {
            const int index = anim_getKeyIndex(key);
            if(index == -1) return;
            const auto& keys = anim_getKeys();
            if(keys.isDuplicateAtIdex(index)) return;
        }
        const int splitId = anim_getKeyIndex(key);
        auto& prev = graph_mKeyPaths[splitId];
        GraphPath next{QPainterPath(), {key->getRelFrame(), prev.fRange.fMax}};
        prev.reset();
        prev.fRange.fMax = key->getRelFrame();
        graph_mKeyPaths.insert(splitId + 1, next);

        graph_constrainCtrlsFrameValues();
    });

    connect(this, &Animator::anim_removedKey, [this](Key * key) {
        if(anim_getKeyAtRelFrame(key->getRelFrame())) return;
        int changeId = anim_getNextKeyId(key->getRelFrame());
        const auto& keys = anim_getKeys();
        if(changeId == -1) changeId = keys.count();
        const int removeId = changeId + 1;

        auto& change = graph_mKeyPaths[changeId];
        auto& remove = graph_mKeyPaths[removeId];

        change.fRange.fMax = remove.fRange.fMax;
        change.reset();

        graph_mKeyPaths.removeAt(removeId);
    });

    graph_mKeyPaths.append({QPainterPath(), FrameRange::EMINMAX});
}

void GraphAnimator::graph_setCtrlsModeForSelectedKeys(const CtrlsMode mode) {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        GetAsGK(key)->setCtrlsMode(mode);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::graph_changeSelectedKeysFrameAndValueStart(
        const QPointF &frameVal) {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        const auto graphKey = GetAsGK(key);
        graphKey->startFrameAndValueTransform();
        graphKey->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::graph_startSelectedKeysTransform() {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        GetAsGK(key)->startFrameAndValueTransform();
    }
}

void GraphAnimator::graph_finishSelectedKeysTransform() {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        GetAsGK(key)->finishFrameAndValueTransform();
    }
}

void GraphAnimator::graph_cancelSelectedKeysTransform() {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        GetAsGK(key)->cancelFrameAndValueTransform();
    }
}

void GraphAnimator::graph_changeSelectedKeysFrameAndValue(
        const QPointF &frameVal) {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        GetAsGK(key)->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::graph_enableCtrlPtsForSelected() {
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        const auto graphKey = GetAsGK(key);
        graphKey->setC1Enabled(true);
        graphKey->setC0Enabled(true);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::graph_drawKeysPath(QPainter * const p,
                                       const QColor &paintColor,
                                       const FrameRange& absFrameRange) {
    const auto relFrameRange = prp_absRangeToRelRange(absFrameRange);
    const auto idRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    if(idRange.fMin == -1 || idRange.fMax == -1) return;
    p->save();
    p->translate(absFrameRange.fMin - relFrameRange.fMin, 0);
    QPen pen(Qt::black, 4);
    pen.setCosmetic(true);
    p->setPen(pen);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        auto& path = graph_mKeyPaths[i];
        if(path.isEmpty()) {
            const auto prevKey = anim_getKeyAtIndex<GraphKey>(i - 1);
            const auto nextKey = anim_getKeyAtIndex<GraphKey>(i);
            path.fPath = graph_getPathForSegment(prevKey, nextKey);
        }
        p->drawPath(path);
    }
    pen.setColor(paintColor);
    pen.setWidth(2);
    p->setPen(pen);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        p->drawPath(graph_mKeyPaths.at(i));
    }
    p->restore();

    p->setPen(Qt::NoPen);

    for(int i = idRange.fMin; i < idRange.fMax; i++) {
        anim_getKeyAtIndex<GraphKey>(i)->drawGraphKey(p, paintColor);
    }
}

void GraphAnimator::graph_getFrameConstraints(
        GraphKey *key, const QrealPointType type,
        qreal &minMoveFrame, qreal &maxMoveFrame) const {
    if(type == QrealPointType::keyPt) {
        minMoveFrame = DBL_MIN;
        maxMoveFrame = DBL_MAX;
        return;
    }
    const qreal keyFrame = key->getAbsFrame();

    qreal startMinMoveFrame;
    qreal endMaxMoveFrame;
    const int keyId = anim_getKeyIndex(key);

    const auto& keys = anim_getKeys();
    if(keyId == keys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000;
    } else {
        endMaxMoveFrame = keys.atId(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000;
    } else {
        const auto& prevKey = keys.atId(keyId - 1);
        startMinMoveFrame = prevKey->getAbsFrame();
    }

    if(key->getCtrlsMode() == CtrlsMode::symmetric) {
        if(type == QrealPointType::c1Pt) {
            minMoveFrame = keyFrame;
            maxMoveFrame = 2*keyFrame - startMinMoveFrame;
            maxMoveFrame = qMin(endMaxMoveFrame, maxMoveFrame);
        } else {
            minMoveFrame = 2*keyFrame - endMaxMoveFrame;
            minMoveFrame = qMax(startMinMoveFrame, minMoveFrame);
            maxMoveFrame = keyFrame;
        }
    } else {
        if(type == QrealPointType::c1Pt) {
            minMoveFrame = keyFrame;
            maxMoveFrame = endMaxMoveFrame;
        } else {
            minMoveFrame = startMinMoveFrame;
            maxMoveFrame = keyFrame;
        }
    }
}

void GraphAnimator::graph_getFrameValueConstraints(
        GraphKey *key, const QrealPointType type,
        qreal &minMoveFrame, qreal &maxMoveFrame,
        qreal &minMoveValue, qreal &maxMoveValue) const {
    graph_getFrameConstraints(key, type, minMoveFrame, maxMoveFrame);
    graph_getValueConstraints(key, type, minMoveValue, maxMoveValue);
}

IdRange GraphAnimator::graph_relFrameRangeToGraphPathIdRange(
        const FrameRange &relFrameRange) const {
    const auto comp1 = [](const GraphPath& a, const int& relFrame) {
        return a.fRange.fMax < relFrame;
    };
    const auto it1 = std::lower_bound(graph_mKeyPaths.begin(),
                                      graph_mKeyPaths.end(),
                                      relFrameRange.fMin, comp1);

    const auto comp2 = [](const GraphPath& a, const int& relFrame) {
        return relFrame == FrameRange::EMAX ?
                    a.fRange.fMax < relFrame :
                    a.fRange.fMax <= relFrame;
    };
    const auto it2 = std::lower_bound(graph_mKeyPaths.begin(),
                                      graph_mKeyPaths.end(),
                                      relFrameRange.fMax, comp2);
    return {static_cast<int>(std::distance(graph_mKeyPaths.begin(), it1)),
            static_cast<int>(std::distance(graph_mKeyPaths.begin(), it2))};
}

QPainterPath GraphAnimator::graph_getPathForSegment(
        const GraphKey * const prevKey,
        const GraphKey * const nextKey) const {
    QPainterPath path;
    if(prevKey) {
        path.moveTo(prevKey->getRelFrame(),
                    prevKey->getValueForGraph());
        if(nextKey) {
            path.cubicTo(QPointF(prevKey->getC1Frame(),
                                 prevKey->getC1Value()),
                         QPointF(nextKey->getC0Frame(),
                                 nextKey->getC0Value()),
                         QPointF(nextKey->getRelFrame(),
                                 nextKey->getValueForGraph()));
        } else {
            path.lineTo(50000, 50000);
        }
    } else {
        path.moveTo(-50000, -50000);
        if(nextKey) {
            path.lineTo(nextKey->getRelFrame(),
                        nextKey->getValueForGraph());
        } else {
            path.lineTo(50000, 50000);
        }
    }
    return path;
}

void GraphAnimator::prp_afterChangedAbsRange(const FrameRange &range, const bool clip) {
    Animator::prp_afterChangedAbsRange(range, clip);
    graph_updateKeysPath(prp_absRangeToRelRange(range));
}

void GraphAnimator::graph_updateKeysPath(const FrameRange &relFrameRange) {
    auto resetRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    for(int i = resetRange.fMax; i >= resetRange.fMin; i--) {
        graph_mKeyPaths[i].reset();
    }
}

void GraphAnimator::graph_constrainCtrlsFrameValues() {
    GraphKey *prevKey = nullptr;
    const auto& keys = anim_getKeys();
    const int iMax = keys.count() - 1;
    for(int i = 0; i <= iMax; i++) {
        const auto iKey = GetAsGK(keys.atId(i));
        if(prevKey) {
            prevKey->constrainC1MaxFrame(iKey->getRelFrame());
            iKey->constrainC0MinFrame(prevKey->getRelFrame());
        } else {
            iKey->constrainC0MinFrame(-DBL_MAX);
        }
        if(i == iMax) {
            iKey->constrainC1MaxFrame(DBL_MAX);
        }
        qreal startMin; qreal startMax;
        graph_getValueConstraints(iKey, QrealPointType::c0Pt,
                                  startMin, startMax);
        iKey->constrainC0Value(startMin, startMax);

        qreal endMin; qreal endMax;
        graph_getValueConstraints(iKey, QrealPointType::c1Pt,
                                  endMin, endMax);
        iKey->constrainC1Value(endMin, endMax);

        prevKey = iKey;
    }
}

QrealPoint *GraphAnimator::graph_getPointAt(const qreal value,
                                            const qreal frame,
                                            const qreal pixelsPerFrame,
                                            const qreal pixelsPerValUnit) {
    QrealPoint *point = nullptr;
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        point = GetAsGK(key)->mousePress(frame, value,
                                         pixelsPerFrame,
                                         pixelsPerValUnit);
        if(point) break;
    }
    return point;
}

qValueRange GraphAnimator::graph_getMinAndMaxValues() const {
    const auto& keys = anim_getKeys();
    if(keys.isEmpty()) return {0, 0};
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    for(const auto &key : keys) {
        const auto graphKey = GetAsGK(key);
        const qreal keyVal = graphKey->getValueForGraph();
        const qreal startVal = graphKey->getC0Value();
        const qreal endVal = graphKey->getC1Value();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    const qreal margin = qMax(1., (maxVal - minVal)*0.01);
    return {minVal - margin, maxVal + margin};
}

qValueRange GraphAnimator::graph_getMinAndMaxValuesBetweenFrames(
        const int startFrame, const int endFrame) const {
    const auto& keys = anim_getKeys();
    if(!keys.isEmpty()) {
        qreal minVal = 100000.;
        qreal maxVal = -100000.;
        for(const auto &key : keys) {
            int keyAbsFrame = key->getAbsFrame();
            if(keyAbsFrame < startFrame) continue;
            if(keyAbsFrame > endFrame) break;
            qreal keyVal = GetAsGK(key)->getValueForGraph();
            qreal startVal = GetAsGK(key)->getC0Value();
            qreal endVal = GetAsGK(key)->getC1Value();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }

        qreal margin = qMax(1., (maxVal - minVal)*0.01);
        return {minVal - margin, maxVal + margin};
    }
    return {0, 0};
}


void GraphAnimator::gAddKeysInRectToList(const QRectF &frameValueRect,
                                         QList<GraphKey*> &target) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto graphKey = GetAsGK(key);
        if(graphKey->isInsideRect(frameValueRect)) {
            target.append(graphKey);
        }
    }
}

void GraphAnimator::graph_getSelectedSegments(QList<QList<GraphKey*>> &segments) {
//    sortSelectedKeys();
    QList<GraphKey*> currentSegment;
    GraphKey* lastKey = nullptr;
    const auto& selectedKeys = anim_getSelectedKeys();
    for(const auto& key : selectedKeys) {
        const auto graphKey = GetAsGK(key);
        if(!lastKey) {
            lastKey = graphKey;
            currentSegment << lastKey;
            continue;
        }
        if(lastKey->getNextKey() != key) {
            if(currentSegment.count() >= 2)
                segments << currentSegment;
            currentSegment.clear();
        }
        currentSegment << graphKey;
        lastKey = graphKey;
    }
    if(currentSegment.count() >= 2) {
        segments << currentSegment;
    }
}

qreal GraphAnimator::graph_prevKeyWeight(const GraphKey * const prevKey,
                                         const GraphKey * const nextKey,
                                         const qreal frame) const {
    const qreal prevFrame = prevKey->getRelFrame();
    const qreal nextFrame = nextKey->getRelFrame();

    const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                prevKey->getC1Frame(),
                nextKey->getC0Frame(),
                qreal(nextKey->getRelFrame())};
    const qreal t = gTFromX(seg, frame);
    const qreal p0y = prevKey->getValueForGraph();
    const qreal p1y = prevKey->getC1Value();
    const qreal p2y = nextKey->getC0Value();
    const qreal p3y = nextKey->getValueForGraph();
    const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
    const qreal dFrame = nextFrame - prevFrame;
    const qreal pWeight = (iFrame - prevFrame)/dFrame;
    return pWeight;
}
