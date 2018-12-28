#include "graphanimator.h"
#include "key.h"
#include "qrealpoint.h"

GraphAnimator::GraphAnimator(const QString& name) : Animator(name) {}

void GraphAnimator::anim_appendKey(const stdsptr<Key>& newKey,
                              const bool &saveUndoRedo,
                              const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    if(anim_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void GraphAnimator::anim_removeKey(const stdsptr<Key>& keyToRemove,
                                   const bool &saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    if(anim_mSelected) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void GraphAnimator::setCtrlsModeForSelectedKeys(const CtrlsMode &mode) {
    Q_FOREACH(const auto& key, anim_mSelectedKeys) {
        key->setCtrlsMode(mode);
        anim_updateAfterChangedKey(key);
    }
}

void GraphAnimator::changeSelectedKeysFrameAndValueStart(
        const QPointF &frameVal) {
    Q_FOREACH(const auto& key, anim_mSelectedKeys) {
        key->saveCurrentFrameAndValue();
        key->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::changeSelectedKeysFrameAndValue(
        const QPointF &frameVal) {
    Q_FOREACH(const auto& key, anim_mSelectedKeys) {
        key->changeFrameAndValueBy(frameVal);
    }
}

void GraphAnimator::enableCtrlPtsForSelected() {
    Q_FOREACH(const auto& key, anim_mSelectedKeys) {
        key->setEndEnabledForGraph(true);
        key->setStartEnabledForGraph(true);
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
    Q_FOREACH(const auto &key, anim_mKeys) {
        key->drawGraphKey(p, paintColor);
    }

    p->restore();
}

void GraphAnimator::getMinAndMaxMoveFrame(
        Key *key, QrealPoint * const currentPoint,
        qreal &minMoveFrame, qreal &maxMoveFrame) {
    if(currentPoint->isKeyPoint()) return;
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
        if(currentPoint->isEndPoint()) {
            minMoveFrame = keyFrame;
            maxMoveFrame = 2*keyFrame - startMinMoveFrame;
            maxMoveFrame = qMin(endMaxMoveFrame, maxMoveFrame);
        } else {
            minMoveFrame = 2*keyFrame - endMaxMoveFrame;
            minMoveFrame = qMax(startMinMoveFrame, minMoveFrame);
            maxMoveFrame = keyFrame;
        }
    } else {
        if(currentPoint->isEndPoint()) {
            minMoveFrame = keyFrame;
            maxMoveFrame = endMaxMoveFrame;
        } else {
            minMoveFrame = startMinMoveFrame;
            maxMoveFrame = keyFrame;
        }
    }
}

void GraphAnimator::anim_updateKeysPath() {
    mKeysPath = QPainterPath();
    Key *lastKey = nullptr;
    Q_FOREACH(const auto &key, anim_mKeys) {
        int keyFrame = key->getAbsFrame();
        qreal keyValue;
        if(keyFrame == anim_mCurrentAbsFrame) {
            keyValue = anim_mCurrentRelFrame;
        } else {
            keyValue = key->getValueForGraph();
        }
        if(lastKey == nullptr) {
            mKeysPath.moveTo(-5000, keyValue);
            mKeysPath.lineTo(keyFrame, keyValue);
        } else {
            mKeysPath.cubicTo(
                        QPointF(lastKey->getEndFrameForGraph(),
                                lastKey->getEndValueForGraph()),
                        QPointF(key->getStartFrameForGraph(),
                                key->getStartValueForGraph()),
                        QPointF(keyFrame, keyValue));
        }
        lastKey = key.get();
    }
    if(lastKey == nullptr) {
        mKeysPath.moveTo(-5000, -5000);
        mKeysPath.lineTo(5000, 5000);
    } else {
        mKeysPath.lineTo(5000, lastKey->getValueForGraph());
    }
}

void GraphAnimator::anim_constrainCtrlsFrameValues() {
    Key *lastKey = nullptr;
    Q_FOREACH(const auto &key, anim_mKeys) {
        if(lastKey != nullptr) {
            lastKey->constrainEndCtrlMaxFrame(key->getAbsFrame());
            key->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
        }
        lastKey = key.get();
    }
    anim_updateKeysPath();
}

QrealPoint *GraphAnimator::anim_getPointAt(const qreal &value,
                                      const qreal &frame,
                                      const qreal &pixelsPerFrame,
                                      const qreal &pixelsPerValUnit) {
    QrealPoint *point = nullptr;
    Q_FOREACH(const auto &key, anim_mKeys) {
        point = key->mousePress(frame, value,
                                pixelsPerFrame, pixelsPerValUnit);
        if(point != nullptr) {
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
        Q_FOREACH(const stdsptr<Key> &key, anim_mKeys) {
            qreal keyVal = key->getValueForGraph();
            qreal startVal = key->getStartValueForGraph();
            qreal endVal = key->getEndValueForGraph();
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
        Q_FOREACH(const stdsptr<Key> &key, anim_mKeys) {
            int keyAbsFrame = key->getAbsFrame();
            if(keyAbsFrame < startFrame) continue;
            if(keyAbsFrame > endFrame) break;
            qreal keyVal = key->getValueForGraph();
            qreal startVal = key->getStartValueForGraph();
            qreal endVal = key->getEndValueForGraph();
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
                                   QList<Key*> &keys) {
    Q_FOREACH(const auto &key, anim_mKeys) {
        if(key->isInsideRect(frameValueRect)) {
            keys.append(key.get());
        }
    }
}
