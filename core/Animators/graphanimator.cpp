#include "graphanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {}

void GraphAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
    if(graph_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void GraphAnimator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Animator::anim_removeKey(keyToRemove);
    if(graph_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
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
                                 const QColor &paintColor) const {
    p->save();

    QPen pen = QPen(Qt::black, 4.);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawPath(graph_mKeysPath);
    pen.setColor(paintColor);
    pen.setWidthF(2.);
    p->setPen(pen);
    p->drawPath(graph_mKeysPath);

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
        endMaxMoveFrame = anim_mKeys.at(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        Key *prevKey = anim_mKeys.at(keyId - 1).get();
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

void GraphAnimator::graph_updateKeysPath() {
    graph_mKeysPath = QPainterPath();
    Key *lastKey = nullptr;
    for(const auto &key : anim_mKeys) {
        int keyFrame = key->getAbsFrame();
        qreal keyValue;
        if(keyFrame == anim_mCurrentAbsFrame) {
            keyValue = anim_mCurrentRelFrame;
        } else {
            keyValue = GetAsGK(key)->getValueForGraph();
        }
        if(!lastKey) {
            graph_mKeysPath.moveTo(-5000, keyValue);
            graph_mKeysPath.lineTo(keyFrame, keyValue);
        } else {
            graph_mKeysPath.cubicTo(
                        QPointF(GetAsGK(lastKey)->getEndFrame(),
                                GetAsGK(lastKey)->getEndValue()),
                        QPointF(GetAsGK(key)->getStartFrame(),
                                GetAsGK(key)->getStartValue()),
                        QPointF(keyFrame, keyValue));
        }
        lastKey = key.get();
    }
    if(!lastKey) {
        graph_mKeysPath.moveTo(-5000, -5000);
        graph_mKeysPath.lineTo(5000, 5000);
    } else {
        graph_mKeysPath.lineTo(5000, GetAsGK(lastKey)->getValueForGraph());
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
    graph_updateKeysPath();
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

ValueRange GraphAnimator::graph_getMinAndMaxValues() const {
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

ValueRange GraphAnimator::graph_getMinAndMaxValuesBetweenFrames(
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
