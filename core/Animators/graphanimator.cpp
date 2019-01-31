#include "graphanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {}

void GraphAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
    if(anim_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void GraphAnimator::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Animator::anim_removeKey(keyToRemove);
    if(anim_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void GraphAnimator::setCtrlsModeForSelectedKeys(const CtrlsMode &mode) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->setCtrlsMode(mode);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::changeSelectedKeysFrameAndValueStart(
        const QPointF &frameVal) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->saveCurrentFrameAndValue();
        GetAsGK(key)->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::changeSelectedKeysFrameAndValue(
        const QPointF &frameVal) {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::enableCtrlPtsForSelected() {
    for(const auto& key : anim_mSelectedKeys) {
        GetAsGK(key)->setEndEnabledForGraph(true);
        GetAsGK(key)->setStartEnabledForGraph(true);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::drawKeysPath(QPainter * const p,
                                 const QColor &paintColor) const {
    p->save();

    QPen pen = QPen(Qt::black, 4.);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawPath(mKeysPath);
    pen.setColor(paintColor);
    pen.setWidthF(2.);
    p->setPen(pen);
    p->drawPath(mKeysPath);

    p->setPen(Qt::NoPen);
    for(const auto &key : anim_mKeys) {
        GetAsGK(key)->drawGraphKey(p, paintColor);
    }

    p->restore();
}

void GraphAnimator::getFrameConstraints(
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

void GraphAnimator::getFrameValueConstraints(
        GraphKey *key, const QrealPointType& type,
        qreal &minMoveFrame, qreal &maxMoveFrame,
        qreal &minMoveValue, qreal &maxMoveValue) const {
    getFrameConstraints(key, type, minMoveFrame, maxMoveFrame);
    getValueConstraints(key, type, minMoveValue, maxMoveValue);
}

void GraphAnimator::anim_updateKeysPath() {
    mKeysPath = QPainterPath();
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
            mKeysPath.moveTo(-5000, keyValue);
            mKeysPath.lineTo(keyFrame, keyValue);
        } else {
            mKeysPath.cubicTo(
                        QPointF(GetAsGK(lastKey)->getEndFrame(),
                                GetAsGK(lastKey)->getEndValue()),
                        QPointF(GetAsGK(key)->getStartFrame(),
                                GetAsGK(key)->getStartValue()),
                        QPointF(keyFrame, keyValue));
        }
        lastKey = key.get();
    }
    if(!lastKey) {
        mKeysPath.moveTo(-5000, -5000);
        mKeysPath.lineTo(5000, 5000);
    } else {
        mKeysPath.lineTo(5000, GetAsGK(lastKey)->getValueForGraph());
    }
}

void GraphAnimator::anim_constrainCtrlsFrameValues() {
    GraphKey *lastKey = nullptr;
    for(const auto &key : anim_mKeys) {
        auto gKey = GetAsGK(key);
        if(lastKey) {
            lastKey->constrainEndCtrlMaxFrame(key->getAbsFrame());
            qreal endMin; qreal endMax;
            getValueConstraints(lastKey, QrealPointType::END_POINT,
                                        endMin, endMax);
            lastKey->constrainEndCtrlValue(endMin, endMax);

            gKey->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
            qreal startMin; qreal startMax;
            getValueConstraints(gKey, QrealPointType::START_POINT,
                                        startMin, startMax);
            gKey->constrainStartCtrlValue(startMin, startMax);
        }
        lastKey = gKey;
    }
    anim_updateKeysPath();
}

QrealPoint *GraphAnimator::anim_getPointAt(const qreal &value,
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

void GraphAnimator::anim_getMinAndMaxValues(
        qreal &minValP, qreal &maxValP) const {
    if(!anim_mKeys.isEmpty()) {
        qreal minVal = 100000.;
        qreal maxVal = -100000.;
        for(const auto &key : anim_mKeys) {
            qreal keyVal = GetAsGK(key)->getValueForGraph();
            qreal startVal = GetAsGK(key)->getStartValue();
            qreal endVal = GetAsGK(key)->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }

        qreal margin = qMax(1., (maxVal - minVal)*0.01);
        minValP = minVal - margin;
        maxValP = maxVal + margin;
    }
}

void GraphAnimator::anim_getMinAndMaxValuesBetweenFrames(
        const int &startFrame, const int &endFrame,
        qreal &minValP, qreal &maxValP) const {
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
        minValP = minVal - margin;
        maxValP = maxVal + margin;
    }
}


void GraphAnimator::addKeysInRectToList(const QRectF &frameValueRect,
                                   QList<GraphKey *> &keys) {
    for(const auto &key : anim_mKeys) {
        if(GetAsGK(key)->isInsideRect(frameValueRect)) {
            keys.append(GetAsGK(key));
        }
    }
}

void GraphAnimator::getSelectedSegments(QList<QList<GraphKey*>> &segments) {
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
