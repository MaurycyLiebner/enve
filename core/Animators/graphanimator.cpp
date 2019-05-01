#include "graphanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {
    graph_mKeyPaths.append(QPainterPath());
}

void GraphAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    const int keyId = getInsertIdForKeyRelFrame(newKey->getRelFrame());
    graph_mKeyPaths.insert(keyId + 1, QPainterPath());
    Animator::anim_appendKey(newKey);
    graph_constrainCtrlsFrameValues();
}

void GraphAnimator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    const int keyId = anim_getKeyIndex(keyToRemove.get()) + 1;
    graph_mKeyPaths.removeAt(keyId);
    Animator::anim_removeKey(keyToRemove);
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
    p->save();

    const auto relFrameRange = prp_absRangeToRelRange(absFrameRange);
    const auto idRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    QPen pen = QPen(Qt::black, 4);
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
    for(const auto &key : anim_mKeys) {
        GetAsGK(key)->drawGraphKey(p, paintColor);
    }

    p->restore();
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

void GraphAnimator::graph_updateKeyPathWithId(const int& id) {
    if(id < 0 || id >= graph_mKeyPaths.count()) return;
    const auto prevKey = anim_getKeyAtIndex<GraphKey>(id - 1);
    const auto nextKey = anim_getKeyAtIndex<GraphKey>(id);
    QPainterPath& path = graph_mKeyPaths[id];
    path = QPainterPath();
    if(prevKey) {
        path.moveTo(prevKey->getRelFrame(), prevKey->getValueForGraph());
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
            path.lineTo(nextKey->getRelFrame(), nextKey->getValueForGraph());
        } else {
            path.lineTo(50000, 50000);
        }
    }
}

IdRange GraphAnimator::graph_relFrameRangeToGraphPathIdRange(
        const FrameRange &relFrameRange) const {
    const int firstKeyId = anim_getPrevKeyId(relFrameRange.fMin + 1);
    const int lastKeyId = anim_getNextKeyId(relFrameRange.fMax - 1);

    const int maxGraphPathId = graph_mKeyPaths.count() - 1;
    const int firstGraphPathId = clamp(firstKeyId + 1, 0, maxGraphPathId);
    const int lastGraphPathId = clamp(lastKeyId, firstGraphPathId,
                                      maxGraphPathId);
    return {firstGraphPathId, lastGraphPathId};
}

void GraphAnimator::graph_updateKeysPath(const FrameRange &relFrameRange) {
    const auto idRange = graph_relFrameRangeToGraphPathIdRange(relFrameRange);
    for(int i = idRange.fMin; i <= idRange.fMax; i++) {
        graph_updateKeyPathWithId(i);
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
