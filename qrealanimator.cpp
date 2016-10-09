#include "qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"
#include "animationdockwidget.h"
#include "boxeslist.h"
#include <QMenu>

QrealAnimator::QrealAnimator() : ConnectedToMainWindow()
{
}

void QrealAnimator::setValueRange(qreal minVal, qreal maxVal)
{
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    setCurrentValue(mCurrentValue);
}

void QrealAnimator::getKeysInRect(QRectF selectionRect,
                                  int minViewedFrame,
                                  qreal pixelsPerFrame,
                                  QList<QrealKey*> *keysList) {
    int keyRectFramesSpan = KEY_RECT_SIZE/pixelsPerFrame;
    int selLeftFrame = selectionRect.left()/pixelsPerFrame + minViewedFrame -
            keyRectFramesSpan;
    int selRightFrame = selectionRect.right()/pixelsPerFrame + minViewedFrame -
            keyRectFramesSpan;
    for(int i = selRightFrame; i >= selLeftFrame; i--) {
        QrealKey *keyAtPos = getKeyAtFrame(i);
        if(keyAtPos != NULL) {
            keysList->append(keyAtPos);
        }
    }
}
void QrealAnimator::addAllKeysToComplexAnimator()
{
    if(mParentAnimator == NULL) return;
    foreach(QrealKey *key, mKeys) {
        mParentAnimator->addChildQrealKey(key);
    }
}

void QrealAnimator::removeAllKeysFromComplexAnimator()
{
    if(mParentAnimator == NULL) return;
    foreach(QrealKey *key, mKeys) {
        mParentAnimator->removeChildQrealKey(key);
    }
}

bool QrealAnimator::hasKeys()
{
    return !mKeys.isEmpty();
}

void QrealAnimator::incAllValues(qreal valInc)
{
    foreach(QrealKey *key, mKeys) {
        key->incValue(valInc);
    }
    incCurrentValue(valInc);
}

QString QrealAnimator::getName()
{
    return mName;
}

void QrealAnimator::setName(QString newName)
{
    mName = newName;
}

bool QrealAnimator::isKeyOnCurrentFrame() {
    return mKeyOnCurrentFrame;
}

bool QrealAnimator::isBoxesListDetailVisible()
{
    return mBoxesListDetailVisible;
}

void QrealAnimator::setBoxesListDetailVisible(bool bT)
{
    mBoxesListDetailVisible = bT;
}

qreal QrealAnimator::getBoxesListHeight()
{
    return LIST_ITEM_HEIGHT;
}

QString QrealAnimator::getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::drawBoxesList(QPainter *p,
                                  qreal drawX, qreal drawY)
{
    if(mIsCurrentAnimator) {
        p->fillRect(drawX, drawY,
                    LIST_ITEM_MAX_WIDTH - drawX, LIST_ITEM_HEIGHT,
                    QColor(255, 255, 255, 125));
    }
    if(mIsRecording) {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_RECORDING);
    } else {
        p->drawPixmap(drawX, drawY, *BoxesList::ANIMATOR_NOT_RECORDING);
    }
    p->setPen(Qt::black);
    drawX += 2*LIST_ITEM_CHILD_INDENT;
    p->drawText(drawX, drawY,
                LIST_ITEM_MAX_WIDTH - 80. - drawX, LIST_ITEM_HEIGHT,
                Qt::AlignVCenter | Qt::AlignLeft,
                getName() );
    p->setPen(Qt::blue);
    p->drawText(LIST_ITEM_MAX_WIDTH - 80., drawY,
                70., LIST_ITEM_HEIGHT,
                Qt::AlignVCenter | Qt::AlignLeft,
                " " + getValueText() );
}
#include <QWidgetAction>
void QrealAnimator::handleListItemMousePress(qreal boxesListX,
                                             qreal relX, qreal relY,
                                             QMouseEvent *event)
{
    if(relX < 0) {
        return;
    }
    Q_UNUSED(relY);
    if(event->button() == Qt::RightButton) {
        QMenu menu;
        menu.addAction("Add Key");
        QAction *selected_action = menu.exec(event->globalPos());
        if(selected_action != NULL)
        {
            if(selected_action->text() == "Add Key")
            {
                if(!mIsRecording) {
                    setRecording(true);
                }
                saveCurrentValueAsKey();
            }
        } else {

        }
    } else {
        if(relX < LIST_ITEM_CHILD_INDENT) {
            setRecording(!mIsRecording);
        } else if(relX < 2*LIST_ITEM_CHILD_INDENT) {
            setBoxesListDetailVisible(!mBoxesListDetailVisible);
        } else if(boxesListX < LIST_ITEM_MAX_WIDTH - 80 || mIsComplexAnimator) {
            getMainWindow()->getKeysView()->
                    graphSetAnimator(this);
        } else {
            QrealAnimatorSpin *spin = new QrealAnimatorSpin(this);
            spin->show();
            spin->move(event->globalPos() +
                       QPoint(LIST_ITEM_MAX_WIDTH - 83. - boxesListX,
                              -relY - 3.));
        }
    }
}

qreal QrealAnimator::getMinPossibleValue() {
    return mMinPossibleVal;
}

qreal QrealAnimator::getMaxPossibleValue() {
    return mMaxPossibleVal;
}

qreal QrealAnimator::getPrefferedValueStep()
{
    return mPrefferedValueStep;
}

void QrealAnimator::setPrefferedValueStep(qreal valueStep)
{
    mPrefferedValueStep = valueStep;
}

void QrealAnimator::removeAllKeys() {
    if(mKeys.isEmpty()) return;
    qreal currentValue = mCurrentValue;
    QList<QrealKey*> keys = mKeys;
    foreach(QrealKey *key, keys) {
        removeKey(key);
    }
    setCurrentValue(currentValue);
}

void QrealAnimator::setRecording(bool rec)
{
    mIsRecording = rec;
    if(rec) {
        saveCurrentValueAsKey();
    } else {
        removeAllKeys();
        updateKeysPath();
    }
    if(mParentAnimator != NULL) {
        mParentAnimator->childAnimatorIsRecordingChanged();
    }
}

bool QrealAnimator::isRecording()
{
    return mIsRecording;
}

void QrealAnimator::updateKeyOnCurrrentFrame()
{
    if(mTraceKeyOnCurrentFrame) {
        mKeyOnCurrentFrame = getKeyAtFrame(mCurrentFrame) != NULL;
    }
}

QrealKey *QrealAnimator::getKeyAtPos(qreal relX, qreal relY,
                                     int minViewedFrame,
                                     qreal pixelsPerFrame) {
    Q_UNUSED(relY);
    qreal relFrame = relX/pixelsPerFrame;
    qreal pressFrame = relFrame + minViewedFrame;
    if(pixelsPerFrame > KEY_RECT_SIZE) {
        int relFrameInt = relFrame;
        if( qAbs((relFrameInt + 0.5)*pixelsPerFrame - relX) > KEY_RECT_SIZE*0.5) {
            return NULL;
        }
    }
    qreal keyRectFramesSpan = 0.5*KEY_RECT_SIZE/pixelsPerFrame;
    int minPossibleKey = pressFrame - keyRectFramesSpan;
    int maxPossibleKey = pressFrame + keyRectFramesSpan;
    QrealKey *keyAtPos = NULL;
    for(int i = maxPossibleKey; i >= minPossibleKey; i--) {
        keyAtPos = getKeyAtFrame(i);
        if(keyAtPos != NULL) {
            return keyAtPos;
        }
    }
    return NULL;
}

void QrealAnimator::clearFromGraphView() {
    MainWindow::getInstance()->getKeysView()->ifIsCurrentAnimatorSetNull(
                this);
}

void QrealAnimator::freezeMinMaxValues()
{
    mMinMaxValuesFrozen = true;
}

void QrealAnimator::setParentAnimator(ComplexAnimator *parentAnimator)
{
    mParentAnimator = parentAnimator;
    if(parentAnimator == NULL) {
        clearFromGraphView();
    }
}

qreal QrealAnimator::getValueAtFrame(int frame) const
{
    int prevId;
    int nextId;
    if(getNextAndPreviousKeyId(&prevId, &nextId, frame) ) {
        if(nextId == prevId) return mKeys.at(nextId)->getValue();
        QrealKey *prevKey = mKeys.at(prevId);
        QrealKey *nextKey = mKeys.at(nextId);
        return getValueAtFrame(frame, prevKey, nextKey);
    } else {
        return mCurrentValue;
    }
}

qreal QrealAnimator::getCurrentValue() const
{
    return mCurrentValue;
}

void QrealAnimator::setCurrentValue(qreal newValue, bool finish)
{
    if(finish) {
        startTransform();
        mCurrentValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);
        finishTransform();
    } else {
        mCurrentValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);
    }

    callUpdater();
}

void QrealAnimator::updateValueFromCurrentFrame()
{
    setCurrentValue(getValueAtFrame(mCurrentFrame) );
}

void QrealAnimator::saveCurrentValueToKey(QrealKey *key)
{
    saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::saveValueToKey(QrealKey *key, qreal value)
{
    key->setValue(value);
    updateKeysPath();

    if(mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }
}

void QrealAnimator::appendKey(QrealKey *newKey) {
    mKeys.append(newKey);
    newKey->incNumberPointers();
    sortKeys();
    //mergeKeysIfNeeded();
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(newKey);
    }

    if(mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }

    updateKeyOnCurrrentFrame();
}

void QrealAnimator::removeKey(QrealKey *removeKey) {
    if(mKeys.removeOne(removeKey) ) {
        if(mParentAnimator != NULL) {
            mParentAnimator->removeChildQrealKey(removeKey);
        }
        removeKey->decNumberPointers();
        sortKeys();

        if(mIsCurrentAnimator) {
            graphScheduleUpdateAfterKeysChanged();
        }

        updateKeyOnCurrrentFrame();
    }
}

void QrealAnimator::moveKeyToFrame(QrealKey *key, int newFrame)
{
    if(mParentAnimator != NULL) {
        mParentAnimator->removeChildQrealKey(key);
    }
    key->setFrame(newFrame);
    if(mParentAnimator != NULL) {
        mParentAnimator->addChildQrealKey(key);
    }
    sortKeys();
    updateKeysPath();

    updateKeyOnCurrrentFrame();
}

void QrealAnimator::setFrame(int frame)
{
    mCurrentFrame = frame;
    updateValueFromCurrentFrame();

    updateKeyOnCurrrentFrame();
}

bool QrealAnimator::getNextAndPreviousKeyId(int *prevIdP, int *nextIdP,
                                            int frame) const {
    if(mKeys.isEmpty()) return false;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    if(frame >= mKeys.last()->getFrame()) {
        *prevIdP = maxId;
        *nextIdP = maxId;
        return true;
    }
    if(frame <= mKeys.first()->getFrame()) {
        *prevIdP = minId;
        *nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        int keyFrame = mKeys.at(guess)->getFrame();
        if(keyFrame > frame) {
            maxId = guess;
        } else if(keyFrame < frame) {
            minId = guess;
        } else {
            *nextIdP = guess;
            *prevIdP = guess;
            return true;
        }
    }

    if(minId == maxId) {
        QrealKey *key = mKeys.at(minId);
        if(key->getFrame() > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(key->getFrame() < frame) {
            if(minId < mKeys.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    *prevIdP = minId;
    *nextIdP = maxId;
    return true;
}

QrealKey *QrealAnimator::getKeyAtFrame(int frame)
{
    if(mKeys.isEmpty()) return NULL;
    if(frame > mKeys.last()->getFrame()) return NULL;
    if(frame < mKeys.first()->getFrame()) return NULL;
    int minId = 0;
    int maxId = mKeys.count() - 1;
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        if(mKeys.at(guess)->getFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }
    QrealKey *minIdKey = mKeys.at(minId);
    if(minIdKey->getFrame() == frame) return minIdKey;
    QrealKey *maxIdKey = mKeys.at(maxId);
    if(maxIdKey->getFrame() == frame) return maxIdKey;
    return NULL;
}

void QrealAnimator::saveValueAtFrameAsKey(int frame) {
    QrealKey *keyAtFrame = getKeyAtFrame(frame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(frame, this, getValueAtFrame(frame));
        appendKey(keyAtFrame);
        updateKeysPath();
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::setTraceKeyOnCurrentFrame(bool bT)
{
    mTraceKeyOnCurrentFrame = bT;
}

void QrealAnimator::saveCurrentValueAsKey()
{
    QrealKey *keyAtFrame = getKeyAtFrame(mCurrentFrame);
    if(keyAtFrame == NULL) {
        keyAtFrame = new QrealKey(mCurrentFrame, this, mCurrentValue);
        appendKey(keyAtFrame);
        updateKeysPath();
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::updateKeysPath()
{
    mKeysPath = QPainterPath();
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        int keyFrame = key->getFrame();
        if(lastKey == NULL) {
            mKeysPath.moveTo(-5000, -key->getValue());
            mKeysPath.lineTo(keyFrame, -key->getValue());
        } else {
            mKeysPath.cubicTo(
                        QPointF(lastKey->getEndValueFrame(),
                                -lastKey->getEndValue()),
                        QPointF(key->getStartValueFrame(),
                                -key->getStartValue()),
                        QPointF(keyFrame, -key->getValue()));
        }
        lastKey = key;
    }
    if(lastKey == NULL) {
        mKeysPath.moveTo(-5000, -mCurrentValue);
        mKeysPath.lineTo(5000, -mCurrentValue);
    } else {
        mKeysPath.lineTo(5000, -lastKey->getValue());
    }
    updateValueFromCurrentFrame();
    setDrawPathUpdateNeeded();
}

bool keysFrameSort(QrealKey *key1, QrealKey *key2)
{
    return key1->getFrame() < key2->getFrame();
}

void QrealAnimator::sortKeys()
{
    qSort(mKeys.begin(), mKeys.end(), keysFrameSort);
}

qreal QrealAnimator::getValueAtFrame(int frame,
                                    QrealKey *prevKey,
                                    QrealKey *nextKey) const
{
    qreal t = tFromX(prevKey->getFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    return calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
}

void QrealAnimator::getMinAndMaxValues(qreal *minValP, qreal *maxValP) {
    if(mMinMaxValuesFrozen) {
        *minValP = mMinPossibleVal;
        *maxValP = mMaxPossibleVal;
        return;
    }
    if(mIsComplexAnimator) {
        *minValP = mMainWindow->getKeysView()->getMinViewedFrame();
        *maxValP = mMainWindow->getKeysView()->getMaxViewedFrame();
        return;
    }
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
        foreach(QrealKey *key, mKeys) {
            qreal keyVal = key->getValue();
            qreal startVal = key->getStartValue();
            qreal endVal = key->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }

        *minValP = minVal;
        *maxValP = maxVal;
    }
}

void QrealAnimator::getMinAndMaxValuesBetweenFrames(
        int startFrame, int endFrame,
        qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
        bool first = true;
        foreach(QrealKey *key, mKeys) {
            int keyFrame = key->getFrame();
            if(keyFrame > endFrame || keyFrame < startFrame) continue;
            if(first) first = false;
            qreal keyVal = key->getValue();
            qreal startVal = key->getStartValue();
            qreal endVal = key->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }
        if(first) {
            int midFrame = (startFrame + endFrame)/2;
            qreal value = getValueAtFrame(midFrame);
            *minValP = value;
            *maxValP = value;
        } else {
            *minValP = minVal;
            *maxValP = maxVal;
        }
    }
}

void QrealAnimator::updateDrawPathIfNeeded(qreal height, qreal margin,
                                   qreal startFrame, qreal minShownVal,
                                   qreal pixelsPerFrame, qreal pixelsPerValUnit)
{
    if(mDrawPathUpdateNeeded ) {
        mDrawPathUpdateNeeded = false;
        QMatrix transform;
        transform.translate(-pixelsPerFrame*(startFrame - 0.5),
                    height + pixelsPerValUnit*minShownVal - margin);
        transform.scale(pixelsPerFrame, pixelsPerValUnit);
        mKeysDrawPath = mKeysPath*transform;
    }
}

void QrealAnimator::drawKeysPath(QPainter *p,
                                 qreal height, qreal margin,
                                 qreal startFrame, qreal minShownVal,
                                 qreal pixelsPerFrame, qreal pixelsPerValUnit)
{
    p->setPen(QPen(Qt::black, 4.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::red, 2.));
    p->drawPath(mKeysDrawPath);
    p->setPen(QPen(Qt::black, 2.));

    p->save();
    p->translate(0., height - margin);
    p->setBrush(Qt::black);
    foreach(QrealKey *key, mKeys) {
        key->drawGraphKey(p,
                  startFrame, minShownVal,
                  pixelsPerFrame, pixelsPerValUnit);
    }
    p->restore();
}

void QrealAnimator::setDrawPathUpdateNeeded()
{
    mDrawPathUpdateNeeded = true;
}

void QrealAnimator::getMinAndMaxMoveFrame(
                                     QrealKey *key, QrealPoint *currentPoint,
                                     qreal *minMoveFrame, qreal *maxMoveFrame) {
    if(currentPoint->isKeyPoint()) return;
    qreal keyFrame = key->getFrame();

    qreal startMinMoveFrame;
    qreal endMaxMoveFrame;
    int keyId = mKeys.indexOf(key);

    if(keyId == mKeys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000.;
    } else {
        endMaxMoveFrame = mKeys.at(keyId + 1)->getFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        QrealKey *prevKey = mKeys.at(keyId - 1);
        startMinMoveFrame = prevKey->getFrame();
    }

    if(key->getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = qMin(endMaxMoveFrame, 2*keyFrame - startMinMoveFrame);
        } else {
            *minMoveFrame = qMax(startMinMoveFrame, 2*keyFrame - endMaxMoveFrame);
            *maxMoveFrame = keyFrame;
        }
    } else {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = endMaxMoveFrame;
        } else {
            *minMoveFrame = startMinMoveFrame;
            *maxMoveFrame = keyFrame;
        }
    }
}

void QrealAnimator::constrainCtrlsFrameValues() {
    QrealKey *lastKey = NULL;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            lastKey->constrainEndCtrlMaxFrame(key->getFrame());
            key->constrainStartCtrlMinFrame(lastKey->getFrame());
        }
        lastKey = key;
    }
    updateKeysPath();
}

qreal QrealAnimator::clampValue(qreal value)
{
    return value;
}

qreal QrealAnimator::getPrevKeyValue(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId == 0) return key->getValue();
    return mKeys.at(keyId - 1)->getValue();
}

qreal QrealAnimator::getNextKeyValue(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId == mKeys.count() - 1) return key->getValue();
    return mKeys.at(keyId + 1)->getValue();
}

bool QrealAnimator::hasPrevKey(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId > 0) return true;
    return false;
}

bool QrealAnimator::hasNextKey(QrealKey *key)
{
    int keyId = mKeys.indexOf(key);
    if(keyId < mKeys.count() - 1) return true;
    return false;
}

void QrealAnimator::retrieveSavedValue()
{
    setCurrentValue(mSavedCurrentValue);
}

void QrealAnimator::incCurrentValue(qreal incBy)
{
    setCurrentValue(mCurrentValue + incBy);
}

void QrealAnimator::startTransform()
{
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void QrealAnimator::finishTransform()
{
    if(mTransformed) {
        addUndoRedo(
                    new ChangeQrealAnimatorValue(mSavedCurrentValue,
                                                 mCurrentValue,
                                                 this) );
        if(mIsRecording) {
            saveCurrentValueAsKey();
        }
        mTransformed = false;
    }
}

void QrealAnimator::cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        retrieveSavedValue();
    }
}

void QrealAnimator::setUpdater(AnimatorUpdater *updater)
{
    mUpdater = updater;
}

void QrealAnimator::callUpdater()
{
    if(mUpdater == NULL) {
        return;
    } else {
        mUpdater->update();
    }
}

void QrealAnimator::drawKeys(QPainter *p, qreal pixelsPerFrame,
                             qreal startX, qreal startY, qreal height,
                             int startFrame, int endFrame)
{
    p->setPen(QPen(Qt::black, 1.));
    foreach(QrealKey *key, mKeys) {
        if(key->getFrame() >= startFrame && key->getFrame() <= endFrame) {
            if(key->isSelected() ) {
                p->setBrush(Qt::yellow);
            } else {
                p->setBrush(Qt::red);
            }
            p->drawRect(
                QRectF(
                    QPointF((key->getFrame() - startFrame + 0.5)*pixelsPerFrame +
                            startX - KEY_RECT_SIZE*0.5,
                            startY + (height - KEY_RECT_SIZE)*0.5 ),
                    QSize(KEY_RECT_SIZE, KEY_RECT_SIZE) ) );
        }
    }
}

void QrealAnimator::multCurrentValue(qreal mult)
{
    setCurrentValue(mCurrentValue*mult);
}

qreal QrealAnimator::getSavedValue()
{
    return mSavedCurrentValue;
}

QrealPoint *QrealAnimator::getPointAt(qreal value, qreal frame,
                                qreal pixelsPerFrame, qreal pixelsPerValUnit) {
    QrealPoint *point = NULL;
    foreach(QrealKey *key, mKeys) {
        point = key->mousePress(frame, value,
                                pixelsPerFrame, pixelsPerValUnit);
        if(point != NULL) {
            break;
        }
    }
    return point;
}

void QrealAnimator::mergeKeysIfNeeded() {
    QrealKey *lastKey = NULL;
    QList<QrealKeyPair> keyPairsToMerge;
    foreach(QrealKey *key, mKeys) {
        if(lastKey != NULL) {
            if(key->getFrame() == lastKey->getFrame() ) {
                if(key->isDescendantSelected()) {
                    keyPairsToMerge << QrealKeyPair(key, lastKey);
                } else {
                    keyPairsToMerge << QrealKeyPair(lastKey, key);
                    key = NULL;
                }
            }
        }
        lastKey = key;
    }
    foreach(QrealKeyPair keyPair, keyPairsToMerge) {
        keyPair.merge();
    }

    updateKeysPath();
}

void QrealAnimator::addKeysInRectToList(QRectF frameValueRect,
                                        QList<QrealKey*> *keys) {
    foreach(QrealKey *key, mKeys) {
        if(key->isInsideRect(frameValueRect)) {
            keys->append(key);
        }
    }
}

void QrealAnimator::setIsCurrentAnimator(bool bT)
{
    mIsCurrentAnimator = bT;
    if(bT) {
        updateKeysPath();
    }
}

QrealAnimatorSpin::QrealAnimatorSpin(QrealAnimator *animator) : QMenu()
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(0, 0, 0, 0); "
                       "color: rgba(0, 0, 0, 0);");
    QWidgetAction *widgetAction = new QWidgetAction(MainWindow::getInstance());
    QDoubleSpinBox *spinBox = new QDoubleSpinBox();
    spinBox->setStyleSheet("background-color: rgba(200, 200, 200); "
                       "color: rgba(0, 0, 255);");
    spinBox->setFixedWidth(70);
    spinBox->setFocus();
    widgetAction->setDefaultWidget(spinBox);
    addAction(widgetAction);

    spinBox->setRange(animator->getMinPossibleValue(),
             animator->getMaxPossibleValue());
    spinBox->setValue(animator->getCurrentValue() );
    spinBox->setSingleStep(animator->getPrefferedValueStep());
    mAnimator = animator;
    mAnimator->startTransform();

    connect(spinBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueEdited(double)));
    connect(spinBox, SIGNAL(editingFinished()),
            this, SLOT(finishValueEdit()));
}

void QrealAnimatorSpin::valueEdited(double newVal)
{
    mAnimator->setCurrentValue(newVal);
    MainWindow::getInstance()->callUpdateSchedulers();
}

void QrealAnimatorSpin::finishValueEdit()
{
    mAnimator->finishTransform();
    MainWindow::getInstance()->callUpdateSchedulers();
    mAnimator->startTransform();
    close();
}
