#include "graphanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {}

void GraphAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
    graph_constrainCtrlsFrameValues();
}

void GraphAnimator::graph_setCtrlsModeForSelectedKeys(const CtrlsMode &mode) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->setCtrlsMode(mode);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::graph_changeSelectedKeysFrameAndValueStart(
        const QPointF &frameVal) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->saveCurrentFrameAndValue();
        GetAsGK(key)->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::graph_changeSelectedKeysFrameAndValue(
        const QPointF &frameVal) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::graph_enableCtrlPtsForSelected() {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->setEndEnabledForGraph(true);
        GetAsGK(key)->setStartEnabledForGraph(true);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::graph_drawKeysPath(QPainter * const p,
                                       const QColor &paintColor,
                                       const FrameRange& absFrameRange) const {
    const auto relFrameRange = prp_absRangeToRelRange(absFrameRange);
    const auto idRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    if(idRange.fMin == -1 || idRange.fMax == -1) return;
    QPen pen(Qt::black, 4);
    pen.setCosmetic(true);
    p->setPen(pen);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        p->drawPath(graph_mKeyPaths.at(i));
    }
    pen.setColor(paintColor);
    pen.setWidth(2);
    p->setPen(pen);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        p->drawPath(graph_mKeyPaths.at(i));
    }

    p->setPen(Qt::NoPen);

    for(int i = idRange.fMin; i < idRange.fMax; i++) {
        anim_getKeyAtIndex<GraphKey>(i)->drawGraphKey(p, paintColor);
    }
}

void GraphAnimator::graph_getFrameConstraints(
        GraphKey *key, const QrealPointType& type,
        qreal &minMoveFrame, qreal &maxMoveFrame) const {
    if(type == QrealPointType::KEY_POINT) {
        minMoveFrame = DBL_MIN;
        maxMoveFrame = DBL_MAX;
        return;
    }
    qreal keyFrame = key->getAbsFrame();

    qreal startMinMoveFrame;
    qreal endMaxMoveFrame;
    int keyId = anim_getKeyIndex(key);

    if(keyId == anim_mKeys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000.;
    } else {
        endMaxMoveFrame = anim_mKeys.atId(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        Key *prevKey = anim_mKeys.atId(keyId - 1);
        startMinMoveFrame = prevKey->getAbsFrame();
    }

    if(key->getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        if(type == QrealPointType::END_POINT) {
            minMoveFrame = keyFrame;
            maxMoveFrame = 2*keyFrame - startMinMoveFrame;
            maxMoveFrame = qMin(endMaxMoveFrame, maxMoveFrame);
        } else {
            minMoveFrame = 2*keyFrame - endMaxMoveFrame;
            minMoveFrame = qMax(startMinMoveFrame, minMoveFrame);
            maxMoveFrame = keyFrame;
        }
    } else {
        if(type == QrealPointType::END_POINT) {
            minMoveFrame = keyFrame;
            maxMoveFrame = endMaxMoveFrame;
        } else {
            minMoveFrame = startMinMoveFrame;
            maxMoveFrame = keyFrame;
        }
    }
}

void GraphAnimator::graph_getFrameValueConstraints(
        GraphKey *key, const QrealPointType& type,
        qreal &minMoveFrame, qreal &maxMoveFrame,
        qreal &minMoveValue, qreal &maxMoveValue) const {
    graph_getFrameConstraints(key, type, minMoveFrame, maxMoveFrame);
    graph_getValueConstraints(key, type, minMoveValue, maxMoveValue);
}

IdRange GraphAnimator::graph_relFrameRangeToGraphPathIdRange(
        const FrameRange &relFrameRange) const {
    return {idForFrame(relFrameRange.fMin), idForFrame(relFrameRange.fMax)};
}

QPainterPath GraphAnimator::graph_getPathForSegment(
        const GraphKey * const prevKey,
        const GraphKey * const nextKey) const {
    QPainterPath path;
    if(prevKey) {
        path.moveTo(prevKey->getRelFrame(),
                    prevKey->getValueForGraph());
        if(nextKey) {
            path.cubicTo(QPointF(prevKey->getEndFrame(),
                                 prevKey->getEndValue()),
                         QPointF(nextKey->getStartFrame(),
                                 nextKey->getStartValue()),
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

void GraphAnimator::graph_updateKeysPath(const FrameRange &relFrameRange) {
    const auto prevKeyId = anim_getPrevKeyId(relFrameRange.fMin);
    auto prevKey = anim_getKeyAtIndex<GraphKey>(prevKeyId);
    int lastKeyId = anim_getNextKeyId(relFrameRange.fMax);
    if(lastKeyId == -1) lastKeyId = anim_mKeys.count();

    const auto removeRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    for(int i = removeRange.fMax; i >= removeRange.fMin; i--) {
        graph_mKeyPaths.removeAt(i);
    }

    int currPathIndex = clamp(removeRange.fMin, 0, graph_mKeyPaths.count());
    for(int i = prevKeyId + 1; i <= lastKeyId; i++) {
        const auto iKey = anim_getKeyAtIndex<GraphKey>(i);
        const auto path = graph_getPathForSegment(prevKey, iKey);
        const int prevRelFrame = prevKey ? prevKey->getRelFrame() : FrameRange::EMIN;
        const int iRelFrame = iKey ? iKey->getRelFrame() : FrameRange::EMAX;
        const GraphPath graphPath{path, prevRelFrame, iRelFrame};
        graph_mKeyPaths.insert(currPathIndex++, graphPath);
        prevKey = iKey;
    }
}

void GraphAnimator::graph_constrainCtrlsFrameValues() {
    GraphKey *lastKey = nullptr;
    for(const auto &key : anim_mKeys) {
        auto gKey = GetAsGK(key);
        if(lastKey) {
            lastKey->constrainEndCtrlMaxFrame(key->getAbsFrame());
            qreal endMin; qreal endMax;
            graph_getValueConstraints(lastKey, QrealPointType::END_POINT,
                                        endMin, endMax);
            lastKey->constrainEndCtrlValue(endMin, endMax);

            gKey->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
            qreal startMin; qreal startMax;
            graph_getValueConstraints(gKey, QrealPointType::START_POINT,
                                        startMin, startMax);
            gKey->constrainStartCtrlValue(startMin, startMax);
        }
        lastKey = gKey;
    }
}

QrealPoint *GraphAnimator::graph_getPointAt(const qreal &value,
                                      const qreal &frame,
                                      const qreal &pixelsPerFrame,
                                      const qreal &pixelsPerValUnit) {
    QrealPoint *point = nullptr;
    for(const auto &key : anim_mKeys) {
        point = GetAsGK(key)->mousePress(frame, value,
                                pixelsPerFrame, pixelsPerValUnit);
        if(point) {
            break;
        }
    }
    return point;
}

qValueRange GraphAnimator::graph_getMinAndMaxValues() const {
    if(anim_mKeys.isEmpty()) return {0, 0};
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    for(const auto &key : anim_mKeys) {
        const qreal keyVal = GetAsGK(key)->getValueForGraph();
        const qreal startVal = GetAsGK(key)->getStartValue();
        const qreal endVal = GetAsGK(key)->getEndValue();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    const qreal margin = qMax(1., (maxVal - minVal)*0.01);
    return {minVal - margin, maxVal + margin};
}

qValueRange GraphAnimator::graph_getMinAndMaxValuesBetweenFrames(
        const int &startFrame, const int &endFrame) const {
    if(!anim_mKeys.isEmpty()) {
        qreal minVal = 100000.;
        qreal maxVal = -100000.;
        for(const auto &key : anim_mKeys) {
            int keyAbsFrame = key->getAbsFrame();
            if(keyAbsFrame < startFrame) continue;
            if(keyAbsFrame > endFrame) break;
            qreal keyVal = GetAsGK(key)->getValueForGraph();
            qreal startVal = GetAsGK(key)->getStartValue();
            qreal endVal = GetAsGK(key)->getEndValue();
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


void GraphAnimator::graph_addKeysInRectToList(const QRectF &frameValueRect,
                                   QList<GraphKey *> &keys) {
    for(const auto &key : anim_mKeys) {
        if(GetAsGK(key)->isInsideRect(frameValueRect)) {
            keys.append(GetAsGK(key));
        }
    }
}

void GraphAnimator::graph_getSelectedSegments(QList<QList<GraphKey*>> &segments) {
//    sortSelectedKeys();
    QList<GraphKey*> currentSegment;
    GraphKey* lastKey = nullptr;
    for(const auto& key : anim_mSelectedKeys) {
        if(!lastKey) {
            lastKey = GetAsGK(key);
            continue;
        }
        if(lastKey->getNextKey() != key) {
            if(currentSegment.count() >= 2) {
                segments << currentSegment;
            }
            currentSegment.clear();
        }
        currentSegment << GetAsGK(key);
        lastKey = GetAsGK(key);
    }
    if(currentSegment.count() >= 2) {
        segments << currentSegment;
    }
}

qreal GraphAnimator::prevKeyWeight(const GraphKey * const prevKey,
                                   const GraphKey * const nextKey,
                                   const qreal &frame) const {
    const qreal prevFrame = prevKey->getRelFrame();
    const qreal nextFrame = nextKey->getRelFrame();

    const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                prevKey->getEndFrame(),
                nextKey->getStartFrame(),
                qreal(nextKey->getRelFrame())};
    const qreal t = gTFromX(seg, frame);
    const qreal p0y = prevKey->getValueForGraph();
    const qreal p1y = prevKey->getEndValue();
    const qreal p2y = nextKey->getStartValue();
    const qreal p3y = nextKey->getValueForGraph();
    const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
    const qreal dFrame = nextFrame - prevFrame;
    const qreal pWeight = (iFrame - prevFrame)/dFrame;
    return pWeight;
}
