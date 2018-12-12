#include "keysview.h"
#include "qrealkey.h"
#include "clipboardcontainer.h"
#include <QPainter>
#include <QMenu>
#include "mainwindow.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "durationrectangle.h"
#include "global.h"
#include "pointhelpers.h"
#include "Animators/qrealanimator.h"
#include "canvaswindow.h"
#include "durationrectsettingsdialog.h"
#include <QApplication>
#include "clipboardcontainer.h"

KeysView::KeysView(BoxScrollWidgetVisiblePart *boxesListVisible,
                   QWidget *parent) :
    QWidget(parent) {
    mMainWindow = MainWindow::getInstance();
    mBoxesListVisible = boxesListVisible;
    mBoxesListVisible->setKeysView(this);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    mScrollTimer = new QTimer(this);
}

void KeysView::setGraphViewed(bool bT) {
    mGraphViewed = bT;
    if(bT) {
        graphResetValueScaleAndMinShown();
    }
    update();
}

void KeysView::middlePress(const QPointF &pressPos) {
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mMiddlePressPos = pressPos;
}

void KeysView::setViewedVerticalRange(const int &top,
                              const int &bottom) {
    mViewedTop = top;
    mViewedBottom = bottom;
    update();
}

void KeysView::middleMove(const QPointF &movePos) {
    qreal diffFrame = (movePos.x() - mMiddlePressPos.x() ) / mPixelsPerFrame;
    int roundX = qRound(diffFrame );
    setFramesRange(mSavedMinViewedFrame - roundX,
                   mSavedMaxViewedFrame - roundX );
}

void KeysView::deleteSelectedKeys() {
    if(mHoveredKey != nullptr) {
        if(mHoveredKey->isSelected()) {
            clearHoveredPoint();
        }
    }
    Q_FOREACH(Key *key, mSelectedKeys) {
        key->deleteKey();
    }
    mSelectedKeys.clear();
}

void KeysView::selectKeysInSelectionRect() {
    QList<Key*> listKeys;
    mBoxesListVisible->getKeysInRect(mSelectionRect.translated(-0.5, 0.),
                                     mPixelsPerFrame,
                                     listKeys);
    Q_FOREACH(Key *key, listKeys) {
        addKeyToSelection(key);
    }
}

void KeysView::resizeEvent(QResizeEvent *e) {
    updatePixelsPerFrame();

    if(mGraphViewed) {
        graphResizeEvent(e);
    }
}

void KeysView::wheelEvent(QWheelEvent *e) {
    if(mGraphViewed) {
        graphWheelEvent(e);
    } else {
        emit wheelEventSignal(e);
        if(mSelecting) {
            QPointF posU = mapFromGlobal(QCursor::pos()) +
                           QPointF(-MIN_WIDGET_HEIGHT/2, 0.);
            mSelectionRect.setBottom(posU.y() + mViewedTop);
        }
        //mBoxesList->handleWheelEvent(e);
    }
}

void KeysView::mousePressEvent(QMouseEvent *e) {
    QPoint posU = e->pos() + QPoint(-MIN_WIDGET_HEIGHT/2, 0);
    if(mGraphViewed) {
        graphMousePressEvent(posU, e->button());
    } else {
        if(e->button() == Qt::MiddleButton) {
            middlePress(posU);
        } else if(e->button() == Qt::LeftButton) {
            if(mIsMouseGrabbing) {
                return;
            }
            mFirstMove = true;
            mLastPressPos = posU;

            mLastPressedKey = mBoxesListVisible->getKeyAtPos(
                                                      posU.x(), posU.y(),
                                                      mPixelsPerFrame,
                                                      mMinViewedFrame);
            if(mLastPressedKey == nullptr) {
                mLastPressedDurationRectangleMovable =
                        mBoxesListVisible->getRectangleMovableAtPos(
                                            posU.x(), posU.y(),
                                            mPixelsPerFrame,
                                            mMinViewedFrame);
                if(mLastPressedDurationRectangleMovable == nullptr) {
                    mSelecting = true;
                    qreal posUXFrame = posU.x()/mPixelsPerFrame + mMinViewedFrame;
                    mSelectionRect.setTopLeft(QPointF(posUXFrame,
                                                      posU.y() + mViewedTop));
                    mSelectionRect.setBottomRight(QPointF(posUXFrame,
                                                          posU.y() + mViewedTop));
                } else {
                    mMovingRect = true;
                }
            } else {
                if(!mMainWindow->isShiftPressed() &&
                    !(mLastPressedKey->isSelected() ||
                      mLastPressedKey->areAllChildrenSelected())) {
                    clearKeySelection();
                }
                if(mMainWindow->isShiftPressed() &&
                    (mLastPressedKey->isSelected() ||
                     mLastPressedKey->areAllChildrenSelected())) {
                    removeKeyFromSelection(mLastPressedKey);
                } else {
                    clearHoveredPoint();
                    addKeyToSelection(mLastPressedKey);

                    mMovingKeys = true;
                    mValueInput.setName("move");
                }
            }
        } else {
            if(mMovingKeys) {
                if(!mFirstMove) {
                    QList<Animator*> animators;
                    Q_FOREACH(Key *key, mSelectedKeys) {
                        key->cancelFrameTransform();
                        Animator *keyAnimator = key->getParentAnimator();
                        if(!animators.contains(keyAnimator)) {
                            animators << keyAnimator;
                        }
                    }
                    foreach(Animator *animator, animators) {
                        animator->anim_mergeKeysIfNeeded();
                    }
                }

                mMoveDFrame = 0;
                mMovingKeys = false;
                mScalingKeys = false;
                mMovingRect = false;
                //setMouseTracking(false);
                mIsMouseGrabbing = false;
                releaseMouse();
            } else {
                auto movable =
                        mBoxesListVisible->getRectangleMovableAtPos(
                                            posU.x(), posU.y(),
                                            mPixelsPerFrame,
                                            mMinViewedFrame);
                if(movable == nullptr) {
                } else if(movable->isDurationRect()) {
                    QMenu menu;
                    menu.addAction("Settings...");
                    QAction *selectedAction = menu.exec(e->globalPos());
                    if(selectedAction != nullptr) {
                        auto durRect = GetAsPtr(movable, DurationRectangle);
                        durRect->openDurationSettingsDialog(this);
                    }
                }
            }
        }
    }

    mValueInput.clearAndDisableInput();
    mMainWindow->callUpdateSchedulers();
}

stdsptr<KeysClipboardContainer> KeysView::getSelectedKeysClipboardContainer() {
    stdsptr<KeysClipboardContainer> container =
            SPtrCreate(KeysClipboardContainer)();
    Q_FOREACH(const qptr<Animator>& anim, mSelectedAnimators) {
        QByteArray keyData;
        QBuffer target(&keyData);
        target.open(QIODevice::WriteOnly);
        anim->writeSelectedKeys(&target);
        target.close();
        container->addTargetAnimator(anim, keyData);
    }
    return container;
}

bool KeysView::KFT_handleKeyEventForTarget(QKeyEvent *event) {
    if(mGraphViewed) {
        return graphProcessFilteredKeyEvent(event);
    } else {
        bool inputHandled = false;
        if(mMovingKeys) {
            if(mValueInput.handleKeyPressEventWhileMouseGrabbing(event)) {
                inputHandled = true;
            }
        }
        if(inputHandled) {
            handleMouseMove(mLastMovePos, QApplication::mouseButtons());
        } else if(event->modifiers() & Qt::ControlModifier &&
           event->key() == Qt::Key_V) {
            if(event->isAutoRepeat()) return false;
            auto cont = mMainWindow->getClipboardContainer(CCT_KEYS);
            KeysClipboardContainer* container =
                    GetAsPtr(cont, KeysClipboardContainer);
            if(container == nullptr) return false;
            container->paste(mMainWindow->getCurrentFrame(), this, true, true);
        } else if(!mSelectedKeys.isEmpty()) {
            if(event->modifiers() & Qt::ControlModifier &&
                      event->key() == Qt::Key_C) {
                if(event->isAutoRepeat()) return false;
                auto container = getSelectedKeysClipboardContainer();
                mMainWindow->replaceClipboard(container);
            } else if(event->key() == Qt::Key_S) {
                if(!mMovingKeys) {
                    mValueInput.setName("scale");
                    mScalingKeys = true;
                    mMovingKeys = true;
                    mFirstMove = true;
                    mLastPressPos = mapFromGlobal(QCursor::pos()) +
                            QPoint(-MIN_WIDGET_HEIGHT/2, 0);;
                    mIsMouseGrabbing = true;
                    //setMouseTracking(true);
                    grabMouse();
                }
            } else if(event->key() == Qt::Key_G) {
                if(!mMovingKeys) {
                    mValueInput.setName("move");
                    mMovingKeys = true;
                    mFirstMove = true;
                    mLastPressPos = mapFromGlobal(QCursor::pos()) +
                            QPoint(-MIN_WIDGET_HEIGHT/2, 0);;
                    mIsMouseGrabbing = true;
                    //setMouseTracking(true);
                    grabMouse();
                }
            } else if(mMainWindow->isShiftPressed() &&
                     event->key() == Qt::Key_D) {
                auto container = getSelectedKeysClipboardContainer();
                int lowestKey = INT_MAX;
                Q_FOREACH(Key *key, mSelectedKeys) {
                    int keyAbsFrame = key->getAbsFrame();
                    if(keyAbsFrame < lowestKey) {
                        lowestKey = keyAbsFrame;
                    }
                }
                container->paste(lowestKey, this, false, true);

                mValueInput.setName("move");
                mMovingKeys = true;
                mFirstMove = true;
                mLastPressPos = mapFromGlobal(QCursor::pos()) +
                        QPoint(-MIN_WIDGET_HEIGHT/2, 0);;
                mIsMouseGrabbing = true;
                //setMouseTracking(true);
                grabMouse();
            } else if(mMainWindow->isCtrlPressed() &&
                      event->key() == Qt::Key_D) {
                auto container = getSelectedKeysClipboardContainer();
                container->paste(mMainWindow->getCurrentFrame(), this, true, true);
             } else if(event->key() == Qt::Key_Delete) {
                deleteSelectedKeys();
                update();
            } else if(event->modifiers() & Qt::ControlModifier &&
                      event->key() == Qt::Key_Right) {
                Q_FOREACH(Key *key, mSelectedKeys) {
                    key->incFrameAndUpdateParentAnimator(1);
                }
            } else if(event->modifiers() & Qt::ControlModifier &&
                      event->key() == Qt::Key_Left) {
                Q_FOREACH(Key *key, mSelectedKeys) {
                    key->incFrameAndUpdateParentAnimator(-1);
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

void KeysView::focusInEvent(QFocusEvent *) {
    KeyFocusTarget::KFT_setCurrentTarget(this);
}

#include "GUI/BoxesList/boxsinglewidget.h"

void KeysView::paintEvent(QPaintEvent *) {
    QPainter p(this);

    if(mGraphViewed) {
        p.fillRect(rect(), QColor(60, 60, 60));
    } else {
        p.fillRect(rect(), QColor(60, 60, 60));
    }

    if(mPixelsPerFrame < 0.001) return;

//    QRect rect1(0, 0, 10, height());
//    QLinearGradient gradient1(QPoint(rect1.left(), 0),
//                              QPoint(rect1.right(), 0));
//    gradient1.setColorAt(0, QColor(30, 30, 30));
//    gradient1.setColorAt(1, QColor(60, 60, 60));

//    QRect rect2(width() - 30, 0, 30, height());
//    QLinearGradient gradient2(QPoint(rect2.left(), 0),
//                              QPoint(rect2.right(), 0));
//    gradient2.setColorAt(0, QColor(60, 60, 60));
//    gradient2.setColorAt(1, QColor(30, 30, 30));

//    p.fillRect(rect1, gradient1);
//    p.fillRect(rect2, gradient2);
//    p.fillRect(0, 0, 10, height(), QColor(30, 30, 30));
//    p.fillRect(width() - 30, 0, 30, height(), QColor(30, 30, 30));


    if(!mGraphViewed) {    
        int currY = MIN_WIDGET_HEIGHT;
        p.setPen(QPen(QColor(40, 40, 40), 1.));
        while(currY < height()) {
            p.drawLine(0, currY, width(), currY);

            currY += MIN_WIDGET_HEIGHT;
        }
    }
    p.translate(MIN_WIDGET_HEIGHT/2, 0);

    p.setPen(QPen(QColor(75, 75, 75), 1.));
    qreal xT = mPixelsPerFrame*0.5;
    int iInc = 1;
    bool mult5 = true;
    while(iInc*mPixelsPerFrame < MIN_WIDGET_HEIGHT/2) {
        if(mult5) {
            iInc *= 5;
        } else {
            iInc *= 2;
        }
    }
    int minFrame = mMinViewedFrame;//mMainWindow->getMinFrame();
    int maxFrame = mMaxViewedFrame;
//    if(mGraphViewed) {
//        while(xT + minFrame*mPixelsPerFrame < 38.) {
//            minFrame++;
//        }
//    }
    minFrame += ceil((-xT)/mPixelsPerFrame);
    minFrame = minFrame - minFrame%iInc - 1;
    maxFrame += floor((width() - 40. - xT)/mPixelsPerFrame) - maxFrame%iInc;
    for(int i = minFrame; i <= maxFrame; i += iInc) {
        qreal xTT = xT + (i - mMinViewedFrame + 1)*mPixelsPerFrame;
        p.drawLine(QPointF(xTT, 0.), QPointF(xTT, height()) );
    }

    if(mMainWindow->getCurrentFrame() <= maxFrame &&
       mMainWindow->getCurrentFrame() >= minFrame) {
        xT = (mMainWindow->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
                mPixelsPerFrame*0.5;
        p.setPen(QPen(Qt::darkGray, 2.));
        p.drawLine(QPointF(xT, 0.), QPointF(xT, height()) );
    }

    p.setPen(QPen(Qt::black, 1.));

    if(mGraphViewed) {
        p.save();
        graphPaint(&p);
        p.restore();
    } else {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        qreal transDFrame = 0.5*MIN_WIDGET_HEIGHT/mPixelsPerFrame;
        qreal frameAtZeroX = mMinViewedFrame - transDFrame;
        int frameAtZeroXi = qFloor(frameAtZeroX);
        p.translate((frameAtZeroXi - mMinViewedFrame)*mPixelsPerFrame, 0.);
        mBoxesListVisible->drawKeys(&p,
                                    mPixelsPerFrame,
                                    frameAtZeroXi,
                                    qCeil(mMaxViewedFrame + 2*transDFrame));
        p.restore();
        if(mSelecting) {
            p.setPen(QPen(Qt::blue, 2., Qt::DotLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(QRectF((mSelectionRect.x() - mMinViewedFrame)*mPixelsPerFrame,
                              mSelectionRect.y() - mViewedTop,
                              mSelectionRect.width()*mPixelsPerFrame,
                              mSelectionRect.height()));
        }
    }

    p.resetTransform();
    mValueInput.draw(&p, height() - MIN_WIDGET_HEIGHT);
    if(hasFocus() ) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4.));
        p.drawRect(0, 0,
                   width(), height());
    }

    p.end();
}

void KeysView::updateHoveredPointFromPos(const QPoint &posU) {
    if(mHoveredKey != nullptr) mHoveredKey->setHovered(false);
    mHoveredKey = mBoxesListVisible->getKeyAtPos(
                                    posU.x(), posU.y(),
                                    mPixelsPerFrame,
                                    mMinViewedFrame);
    if(mHoveredKey != nullptr) {
        mHoveredKey->setHovered(true);
        clearHoveredMovable();
    } else {
        DurationRectangleMovable *lastMovable = mHoveredMovable;
        mHoveredMovable = mBoxesListVisible->getRectangleMovableAtPos(
                            posU.x(), posU.y(),
                            mPixelsPerFrame,
                            mMinViewedFrame);
        if(lastMovable != mHoveredMovable) {
            if(lastMovable != nullptr) {
                lastMovable->setHovered(false);
            }
            if(mHoveredMovable != nullptr) {
                mHoveredMovable->setHovered(true);
                setCursor(mHoveredMovable->getHoverCursorShape());
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
        clearHoveredPoint();
    }
}

void KeysView::clearHovered() {
    clearHoveredMovable();
    clearHoveredPoint();
}

void KeysView::clearHoveredPoint() {
    if(mHoveredKey == nullptr) return;
    mHoveredKey->setHovered(false);
    mHoveredKey = nullptr;
}

void KeysView::clearHoveredMovable() {
    if(mHoveredMovable == nullptr) return;
    mHoveredMovable->setHovered(false);
    mHoveredMovable = nullptr;
    setCursor(Qt::ArrowCursor);
}

void KeysView::scrollRight() {
    int pixelInc = 2*MIN_WIDGET_HEIGHT;
    int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame += inc;
    mMaxViewedFrame += inc;
    emit changedViewedFrames(mMinViewedFrame, mMaxViewedFrame);
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() +
                                      QPointF(inc, 0.));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() - inc*mPixelsPerFrame);
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::scrollLeft() {
    int pixelInc = 2*MIN_WIDGET_HEIGHT;
    int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame -= inc;
    mMaxViewedFrame -= inc;
    emit changedViewedFrames(mMinViewedFrame, mMaxViewedFrame);
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() -
                                      QPointF(inc, 0.));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() + inc*mPixelsPerFrame);
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::handleMouseMove(const QPoint &pos,
                               const Qt::MouseButtons &buttons) {
    QPoint posU = pos + QPoint(-MIN_WIDGET_HEIGHT/2, 0);

    if(mIsMouseGrabbing ||
       (buttons & Qt::LeftButton ||
        buttons & Qt::RightButton ||
        buttons & Qt::MiddleButton)) {
        if(mGraphViewed) {
            graphMouseMoveEvent(posU, buttons);
        } else {
            if(buttons & Qt::MiddleButton) {
                middleMove(posU);
                emit changedViewedFrames(mMinViewedFrame,
                                         mMaxViewedFrame);
            } else {
                if(posU.x() < -MIN_WIDGET_HEIGHT/2) {
                    if(!mScrollTimer->isActive()) {
                        connect(mScrollTimer, SIGNAL(timeout()),
                                this, SLOT(scrollLeft()));
                        mScrollTimer->start(300);
                    }
                } else if(posU.x() > width() - MIN_WIDGET_HEIGHT) {
                    if(!mScrollTimer->isActive()) {
                        connect(mScrollTimer, SIGNAL(timeout()),
                                this, SLOT(scrollRight()));
                        mScrollTimer->start(300);
                    }
                } else {
                    mScrollTimer->disconnect();
                    mScrollTimer->stop();
                }
                if(mMovingKeys) {
                    if(mFirstMove) {
                        Q_FOREACH(Key *key, mSelectedKeys) {
                            key->startFrameTransform();
                        }
                    }
                    if(mScalingKeys) {
                        qreal keysScale = 1. + (posU.x() - mLastPressPos.x())/150.;
                        if(mValueInput.inputEnabled()) {
                            keysScale = mValueInput.getValue();
                        }
                        Q_FOREACH(Key *key, mSelectedKeys) {
                            key->scaleFrameAndUpdateParentAnimator(
                                        mMainWindow->getCurrentFrame(),
                                        keysScale);
                        }
                    } else {

                        int dFrame =
                                qRound((posU.x() - mLastPressPos.x())/
                                       mPixelsPerFrame);
                        if(mValueInput.inputEnabled()) {
                            dFrame = mValueInput.getValue();
                        }
                        int dDFrame = dFrame - mMoveDFrame;

                        if(dDFrame != 0) {
                            mMoveDFrame = dFrame;
                            Q_FOREACH(Key *key, mSelectedKeys) {
                                key->incFrameAndUpdateParentAnimator(dDFrame);
                            }
                        }
                    }
                } else if(mMovingRect) {
                    auto canvasWindow = mMainWindow->getCanvasWindow();
                    if(mFirstMove) {
                        if(mLastPressedDurationRectangleMovable != nullptr) {
                            if(!mLastPressedDurationRectangleMovable->isSelected()) {
                                mLastPressedDurationRectangleMovable->pressed(
                                            mMainWindow->isShiftPressed());
                            }
                            if(mLastPressedDurationRectangleMovable->isDurationRect()) {
                                canvasWindow->startDurationRectPosTransformForAllSelected();
                            } else if(mLastPressedDurationRectangleMovable->isMaxFrame()) {
                                canvasWindow->startMaxFramePosTransformForAllSelected();
                            } else if(mLastPressedDurationRectangleMovable->isMinFrame()) {
                                canvasWindow->startMinFramePosTransformForAllSelected();
                            }
                        }
                    }
                    int dFrame =
                            qRound((posU.x() - mLastPressPos.x())/
                                   mPixelsPerFrame);
                    if(mValueInput.inputEnabled()) {
                        dFrame = mValueInput.getValue();
                    }
                    int dDFrame = dFrame - mMoveDFrame;

                    if(dDFrame != 0) {
                        mMoveDFrame = dFrame;
                        if(mLastPressedDurationRectangleMovable->isDurationRect()) {
                            canvasWindow->moveDurationRectForAllSelected(dDFrame);
                        } else if(mLastPressedDurationRectangleMovable->isMaxFrame()) {
                            canvasWindow->moveMaxFrameForAllSelected(dDFrame);
                        } else if(mLastPressedDurationRectangleMovable->isMinFrame()) {
                            canvasWindow->moveMinFrameForAllSelected(dDFrame);
                        }
//                        mLastPressedDurationRectangleMovable->changeFramePosBy(
//                                    dDFrame);
                    }
                } else if(mSelecting) {
                    qreal posUXFrame = posU.x()/mPixelsPerFrame + mMinViewedFrame;
                    mSelectionRect.setBottomRight(
                                    QPointF(posUXFrame, posU.y() + mViewedTop));
                }
                mFirstMove = false;
            }
        }
    } else {
        updateHoveredPointFromPos(posU);
    }

    mMainWindow->callUpdateSchedulers();
    mLastMovePos = pos;
}

void KeysView::mouseMoveEvent(QMouseEvent *event) {
    handleMouseMove(event->pos(), event->buttons());
}

void KeysView::mouseReleaseEvent(QMouseEvent *e) {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    if(mGraphViewed) {
        graphMouseReleaseEvent(e->button());
    } else {
        if(e->button() == Qt::LeftButton) {
            if(mSelecting) {
                if(mFirstMove) {
                    if(!mMainWindow->isShiftPressed()) {
                        clearKeySelection();
                    }
                } else {
                    if(mSelectionRect.left() > mSelectionRect.right()) {
                        qreal rightT = mSelectionRect.right();
                        qreal leftT = mSelectionRect.left();
                        mSelectionRect.setLeft(rightT);
                        mSelectionRect.setRight(leftT);
                    }
                    if(mSelectionRect.top() > mSelectionRect.bottom()) {
                        qreal bottomT = mSelectionRect.bottom();
                        qreal topT = mSelectionRect.top();
                        mSelectionRect.setTop(bottomT);
                        mSelectionRect.setBottom(topT);
                    }
                    if(!mMainWindow->isShiftPressed()) {
                        clearKeySelection();
                    }
                    selectKeysInSelectionRect();
                }
                mSelecting = false;
            } else if(mMovingKeys) {
                if(mFirstMove) {
                    if(!mMainWindow->isShiftPressed()) {
                        clearKeySelection();
                        addKeyToSelection(mLastPressedKey);
                    }
                }
                QList<Animator*> parentAnimators;
                Q_FOREACH(Key *key, mSelectedKeys) {
                    key->finishFrameTransform();
                    if(parentAnimators.contains(
                                key->getParentAnimator()) ) continue;
                    parentAnimators << key->getParentAnimator();
                }
                Q_FOREACH(Animator *animator, parentAnimators) {
                    animator->anim_mergeKeysIfNeeded();
                }

                mMoveDFrame = 0;
                mMovingKeys = false;
                mScalingKeys = false;
                mIsMouseGrabbing = false;
                //setMouseTracking(false);
                //releaseMouse();
            } else if(mMovingRect) {
                if(mFirstMove) {
                    if(mLastPressedDurationRectangleMovable != nullptr) {
                        mLastPressedDurationRectangleMovable->pressed(
                                    mMainWindow->isShiftPressed());
                    }
                } else {
                    mMoveDFrame = 0;
                    mMovingRect = false;
                    auto canvasWindow = mMainWindow->getCanvasWindow();
                    if(mLastPressedDurationRectangleMovable->isDurationRect()) {
                        canvasWindow->finishDurationRectPosTransformForAllSelected();
                    } else if(mLastPressedDurationRectangleMovable->isMinFrame()) {
                        canvasWindow->finishMinFramePosTransformForAllSelected();
                    } else if(mLastPressedDurationRectangleMovable->isMaxFrame()) {
                        canvasWindow->finishMaxFramePosTransformForAllSelected();
                    }
                }
            }
        }
    }
    updateHoveredPointFromPos(e->pos() + QPoint(-MIN_WIDGET_HEIGHT/2, 0));
    if(mouseGrabber() == this) releaseMouse();

    mValueInput.clearAndDisableInput();
    mMainWindow->callUpdateSchedulers();
}

void KeysView::setFramesRange(const int &startFrame,
                              const int &endFrame) {
    mMinViewedFrame = startFrame;
    mMaxViewedFrame = endFrame;
    updatePixelsPerFrame();
    if(mGraphViewed) {
        graphUpdateDimensions();
    }
    update();
}

int KeysView::getMinViewedFrame() {
    return mMinViewedFrame;
}

int KeysView::getMaxViewedFrame() {
    return mMaxViewedFrame;
}

qreal KeysView::getPixelsPerFrame() {
    return mPixelsPerFrame;
}

void KeysView::updatePixelsPerFrame() {
    qreal animWidth = width() - 2*MIN_WIDGET_HEIGHT;
    qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

bool selectedKeysSort(const stdptr<Key> &key1,
                      const stdptr<Key> &key2) {
    if(key1->getParentAnimator() == key2->getParentAnimator()) {
        return key1->getRelFrame() < key2->getRelFrame();
    }
    return key1->getParentAnimator() < key2->getParentAnimator();
}

void KeysView::sortSelectedKeys() {
    std::sort(mSelectedKeys.begin(), mSelectedKeys.end(), selectedKeysSort);
}

void KeysView::addKeyToSelection(Key *key) {
    key->addToSelection(mSelectedKeys, mSelectedAnimators);
}

void KeysView::removeKeyFromSelection(Key *key) {
    key->removeFromSelection(mSelectedKeys, mSelectedAnimators);
}

void KeysView::clearKeySelection() {
    Q_FOREACH(Key *key, mSelectedKeys) {
        key->setSelected(false);
    }
    mSelectedAnimators.clear();
    mSelectedKeys.clear();
}
