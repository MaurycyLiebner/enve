#include "keysview.h"
#include "Animators/qrealkey.h"
#include "clipboardcontainer.h"
#include <QPainter>
#include <QMenu>
#include "mainwindow.h"
#include "GUI/BoxesList/boxscrollwidgetvisiblepart.h"
#include "durationrectangle.h"
#include "global.h"
#include "pointhelpers.h"
#include "canvaswindow.h"
#include "durationrectsettingsdialog.h"
#include <QApplication>
#include "clipboardcontainer.h"
#include "Animators/qrealpoint.h"

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
        mValueInput.setForce1D(false);
    } else {
        mValueInput.setForce1D(true);
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
    const qreal diffFrame = (movePos.x() - mMiddlePressPos.x())/
                                    mPixelsPerFrame;
    const int roundX = qRound(diffFrame);
    setFramesRange(mSavedMinViewedFrame - roundX,
                   mSavedMaxViewedFrame - roundX);
}

void KeysView::deleteSelectedKeys() {
    if(mHoveredKey) {
        if(mHoveredKey->isSelected()) {
            clearHoveredPoint();
        }
    }
    for(const auto& anim : mSelectedKeysAnimators) {
        anim->deleteSelectedKeys();
    }
}

void KeysView::selectKeysInSelectionRect() {
    if(mGraphViewed) {
        QList<GraphKey*> keysList;
        for(const auto& anim : mGraphAnimators) {
            anim->graph_addKeysInRectToList(mSelectionRect, keysList);
        }
        for(const auto& key : keysList) {
            addKeyToSelection(key);
        }
    } else {
        QList<Key*> listKeys;
        mBoxesListVisible->getKeysInRect(mSelectionRect.translated(-0.5, 0),
                                         mPixelsPerFrame,
                                         listKeys);
        for(const auto& key : listKeys) {
            addKeyToSelection(key);
        }
    }
}

void KeysView::resizeEvent(QResizeEvent *e) {
    updatePixelsPerFrame();
    if(mGraphViewed) graphResizeEvent(e);
}

void KeysView::wheelEvent(QWheelEvent *e) {
    if(mGraphViewed) {
        graphWheelEvent(e);
    } else {
        emit wheelEventSignal(e);
        if(mSelecting) {
            const QPointF posU = mapFromGlobal(QCursor::pos()) +
                           QPointF(-MIN_WIDGET_HEIGHT/2, 0.);
            mSelectionRect.setBottom(posU.y() + mViewedTop);
        }
        //mBoxesList->handleWheelEvent(e);
    }
}

void KeysView::mousePressEvent(QMouseEvent *e) {
    KFT_setFocus();
    const QPoint posU = e->pos() + QPoint(-MIN_WIDGET_HEIGHT/2, 0);
    if(e->button() == Qt::MiddleButton) {
        if(mGraphViewed) graphMiddlePress(posU);
        else  middlePress(posU);
    } else if(e->button() == Qt::LeftButton) {
        if(mIsMouseGrabbing) return;
        mFirstMove = true;
        mLastPressPos = posU;
        if(mGraphViewed) graphMousePress(posU);
        else {
            mLastPressedKey = mBoxesListVisible->getKeyAtPos(
                                                      posU.x(), posU.y(),
                                                      mPixelsPerFrame,
                                                      mMinViewedFrame);
            if(!mLastPressedKey) {
                mLastPressedMovable =
                        mBoxesListVisible->getRectangleMovableAtPos(
                                            posU.x(), posU.y(),
                                            mPixelsPerFrame,
                                            mMinViewedFrame);
                if(!mLastPressedMovable) {
                    mSelecting = true;
                    const qreal posUXFrame = posU.x()/mPixelsPerFrame + mMinViewedFrame;
                    const QPointF xFramePos(posUXFrame,
                                            posU.y() + mViewedTop);
                    mSelectionRect.setTopLeft(xFramePos);
                    mSelectionRect.setBottomRight(xFramePos);
                } else {
                    mMovingRect = true;
                }
            } else {
                if(!mMainWindow->isShiftPressed() &&
                    !mLastPressedKey->isSelected()) {
                    clearKeySelection();
                }
                if(mMainWindow->isShiftPressed() &&
                    mLastPressedKey->isSelected()) {
                    removeKeyFromSelection(mLastPressedKey);
                } else {
                    clearHoveredPoint();
                    addKeyToSelection(mLastPressedKey);

                    mMovingKeys = true;
                    mValueInput.setupMove();
                }
            }
        }
    } else {
        if(mMovingKeys) {
            if(!mFirstMove) {
                if(mGraphViewed) {
                    for(const auto& anim : mGraphAnimators) {
                        anim->graph_cancelSelectedKeysTransform();
                    }
                } else {
                    for(const auto& anim : mSelectedKeysAnimators) {
                        anim->cancelSelectedKeysTransform();
                    }
                }
            }

            mMoveDFrame = 0;
            mMovingKeys = false;
            mScalingKeys = false;
            mMovingRect = false;
            releaseMouseAndDontTrack();
        } else {
            auto movable = mBoxesListVisible->getRectangleMovableAtPos(
                                        posU.x(), posU.y(),
                                        mPixelsPerFrame,
                                        mMinViewedFrame);
            if(!movable) {
            } else if(movable->isDurationRect()) {
                QMenu menu;
                menu.addAction("Settings...");
                const auto selectedAction = menu.exec(e->globalPos());
                if(selectedAction) {
                    const auto durRect = GetAsPtr(movable, DurationRectangle);
                    durRect->openDurationSettingsDialog(this);
                }
            }
        }
    }

    mValueInput.clearAndDisableInput();
    mMainWindow->queScheduledTasksAndUpdate();
}

stdsptr<KeysClipboardContainer> KeysView::getSelectedKeysClipboardContainer() {
    stdsptr<KeysClipboardContainer> container =
            SPtrCreate(KeysClipboardContainer)();
    for(const auto& anim : mSelectedKeysAnimators) {
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
    bool inputHandled = false;
    if(mMovingKeys) {
        if(mValueInput.handleTransormationInputKeyEvent(event)) {
            inputHandled = true;
        }
    }
    const QPoint posU = mapFromGlobal(QCursor::pos()) +
            QPoint(-MIN_WIDGET_HEIGHT/2, 0);
    if(inputHandled) {
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    } else if(graphProcessFilteredKeyEvent(event)) {
    } else if(event->modifiers() & Qt::ControlModifier &&
       event->key() == Qt::Key_V) {
        if(event->isAutoRepeat()) return false;
        auto cont = mMainWindow->getClipboardContainer(CCT_KEYS);
        const auto container =
                GetAsPtr(cont, KeysClipboardContainer);
        if(!container) return false;
        container->paste(mMainWindow->getCurrentFrame(), this, true, true);
    } else if(!mSelectedKeysAnimators.isEmpty()) {
        if(event->modifiers() & Qt::ControlModifier &&
                  event->key() == Qt::Key_C) {
            if(event->isAutoRepeat()) return false;
            auto container = getSelectedKeysClipboardContainer();
            mMainWindow->replaceClipboard(container);
        } else if(event->key() == Qt::Key_S) {
            if(!mMovingKeys) {
                mValueInput.setupScale();
                mScalingKeys = true;
                mMovingKeys = true;
                mFirstMove = true;
                mLastPressPos = posU;
                grabMouseAndTrack();
            }
        } else if(event->key() == Qt::Key_G) {
            if(!mMovingKeys) {
                mValueInput.setupMove();
                mMovingKeys = true;
                mFirstMove = true;
                mLastPressPos = posU;
                grabMouseAndTrack();
            }
        } else if(mMainWindow->isShiftPressed() &&
                 event->key() == Qt::Key_D) {
            auto container = getSelectedKeysClipboardContainer();
            int lowestKey = FrameRange::EMAX;
            for(const auto& anim : mSelectedKeysAnimators) {
                const int animLowest = anim->getLowestAbsFrameForSelectedKey();
                if(animLowest < lowestKey) {
                    lowestKey = animLowest;
                }
            }
            container->paste(lowestKey, this, false, true);

            mValueInput.setupMove();
            mMovingKeys = true;
            mFirstMove = true;
            mLastPressPos = posU;
            grabMouseAndTrack();
        } else if(mMainWindow->isCtrlPressed() &&
                  event->key() == Qt::Key_D) {
            auto container = getSelectedKeysClipboardContainer();
            container->paste(mMainWindow->getCurrentFrame(), this, true, true);
         } else if(event->key() == Qt::Key_Delete) {
            if(mGraphViewed) {
                graphDeletePressed();
            } else {
                deleteSelectedKeys();
            }
            update();
        } else if(event->modifiers() & Qt::ControlModifier &&
                  event->key() == Qt::Key_Right) {
            for(const auto& anim : mSelectedKeysAnimators) {
                anim->incSelectedKeysFrame(1);
            }
        } else if(event->modifiers() & Qt::ControlModifier &&
                  event->key() == Qt::Key_Left) {
            for(const auto& anim : mSelectedKeysAnimators) {
                anim->incSelectedKeysFrame(-1);
            }
        } else return false;
    } else return false;
    return true;
}

void KeysView::focusInEvent(QFocusEvent *) {
    KeyFocusTarget::KFT_setCurrentTarget(this);
}

#include "GUI/BoxesList/boxsinglewidget.h"

void KeysView::paintEvent(QPaintEvent *) {
    QPainter p(this);

    if(mGraphViewed) p.fillRect(rect(), QColor(60, 60, 60));
    else p.fillRect(rect(), QColor(60, 60, 60));

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
        p.setPen(QPen(QColor(40, 40, 40), 1));
        while(currY < height()) {
            p.drawLine(0, currY, width(), currY);
            currY += MIN_WIDGET_HEIGHT;
        }
    }
    p.translate(MIN_WIDGET_HEIGHT/2, 0);

    p.setPen(QPen(QColor(75, 75, 75), 1));
    qreal xT = mPixelsPerFrame*0.5;
    int iInc = 1;
    bool mult5 = true;
    while(iInc*mPixelsPerFrame < MIN_WIDGET_HEIGHT/2) {
        if(mult5) iInc *= 5;
        else iInc *= 2;
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
    maxFrame += floor((width() - 40 - xT)/mPixelsPerFrame) - maxFrame%iInc;
    for(int i = minFrame; i <= maxFrame; i += iInc) {
        const qreal xTT = xT + (i - mMinViewedFrame + 1)*mPixelsPerFrame;
        p.drawLine(QPointF(xTT, 0), QPointF(xTT, height()));
    }

    if(mMainWindow->getCurrentFrame() <= maxFrame &&
       mMainWindow->getCurrentFrame() >= minFrame) {
        xT = (mMainWindow->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
                mPixelsPerFrame*0.5;
        p.setPen(QPen(Qt::darkGray, 2));
        p.drawLine(QPointF(xT, 0), QPointF(xT, height()));
    }

    p.setPen(QPen(Qt::black, 1));

    if(mGraphViewed) {
        p.save();
        graphPaint(&p);
        p.restore();
    } else {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        const qreal transDFrame = 0.5*MIN_WIDGET_HEIGHT/mPixelsPerFrame;
        const qreal frameAtZeroX = mMinViewedFrame - transDFrame;
        const int frameAtZeroXi = qFloor(frameAtZeroX);
        p.translate((frameAtZeroXi - mMinViewedFrame)*mPixelsPerFrame, 0);
        const int maxFrame = qCeil(mMaxViewedFrame + 2*transDFrame);
        const FrameRange viewedFrameRange{frameAtZeroXi, maxFrame};
        mBoxesListVisible->drawKeys(&p, mPixelsPerFrame, viewedFrameRange);
        p.restore();
        if(mSelecting) {
            p.setPen(QPen(Qt::blue, 2, Qt::DotLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(QRectF((mSelectionRect.x() - mMinViewedFrame)*mPixelsPerFrame,
                              mSelectionRect.y() - mViewedTop,
                              mSelectionRect.width()*mPixelsPerFrame,
                              mSelectionRect.height()));
        }
    }

    p.resetTransform();
    if(mMovingKeys) mValueInput.draw(&p, height() - MIN_WIDGET_HEIGHT);
    if(hasFocus()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4));
        p.drawRect(0, 0, width(), height());
    }

    p.end();
}

void KeysView::updateHoveredPointFromPos(const QPoint &posU) {
    if(mHoveredKey) mHoveredKey->setHovered(false);
    mHoveredKey = mBoxesListVisible->getKeyAtPos(
                                    posU.x(), posU.y(),
                                    mPixelsPerFrame,
                                    mMinViewedFrame);
    if(mHoveredKey) {
        mHoveredKey->setHovered(true);
        clearHoveredMovable();
    } else {
        const auto lastMovable = mHoveredMovable;
        mHoveredMovable = mBoxesListVisible->getRectangleMovableAtPos(
                            posU.x(), posU.y(),
                            mPixelsPerFrame,
                            mMinViewedFrame);
        if(lastMovable != mHoveredMovable) {
            if(lastMovable) lastMovable->setHovered(false);
            if(mHoveredMovable) {
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
    if(!mHoveredKey) return;
    mHoveredKey->setHovered(false);
    mHoveredKey = nullptr;
}

void KeysView::clearHoveredMovable() {
    if(!mHoveredMovable) return;
    mHoveredMovable->setHovered(false);
    mHoveredMovable = nullptr;
    setCursor(Qt::ArrowCursor);
}

void KeysView::scrollRight() {
    const int pixelInc = 2*MIN_WIDGET_HEIGHT;
    const int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame += inc;
    mMaxViewedFrame += inc;
    emit changedViewedFrames(mMinViewedFrame, mMaxViewedFrame);
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() +
                                      QPointF(inc, 0));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() - inc*mPixelsPerFrame);
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::scrollLeft() {
    const int pixelInc = 2*MIN_WIDGET_HEIGHT;
    const int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame -= inc;
    mMaxViewedFrame -= inc;
    emit changedViewedFrames(mMinViewedFrame, mMaxViewedFrame);
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() -
                                      QPointF(inc, 0));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() + qFloor(inc*mPixelsPerFrame));
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::handleMouseMove(const QPoint &pos,
                               const Qt::MouseButtons &buttons) {
    const QPoint posU = pos + QPoint(-MIN_WIDGET_HEIGHT/2, 0);

    if(mIsMouseGrabbing ||
       (buttons & Qt::LeftButton ||
        buttons & Qt::RightButton ||
        buttons & Qt::MiddleButton)) {
        if(mGraphViewed && mPressedCtrlPoint && mPressedPoint) {
            qreal value;
            qreal frame;
            graphGetValueAndFrameFromPos(posU, &value, &frame);
            const qreal clampedFrame = clamp(frame, mMinMoveFrame, mMaxMoveFrame);
            const qreal clamedValue = clamp(value, mMinMoveVal, mMaxMoveVal);
            mPressedPoint->moveTo(clampedFrame, clamedValue);
        } else if(buttons & Qt::MiddleButton) {
            if(mGraphViewed) graphMiddleMove(posU);
            else middleMove(posU);
            emit changedViewedFrames(mMinViewedFrame,
                                     mMaxViewedFrame);
        } else {
            if(posU.x() < -MIN_WIDGET_HEIGHT/2) {
                if(!mScrollTimer->isActive()) {
                    connect(mScrollTimer, &QTimer::timeout,
                            this, &KeysView::scrollLeft);
                    mScrollTimer->start(300);
                }
            } else if(posU.x() > width() - MIN_WIDGET_HEIGHT) {
                if(!mScrollTimer->isActive()) {
                    connect(mScrollTimer, &QTimer::timeout,
                            this, &KeysView::scrollRight);
                    mScrollTimer->start(300);
                }
            } else {
                mScrollTimer->disconnect();
                mScrollTimer->stop();
            }
            int dFrame;
            if(mValueInput.inputEnabled()) {
                dFrame = qRound(mValueInput.getValue());
            } else {
                const qreal dX = posU.x() - mLastPressPos.x();
                dFrame = qRound(dX/mPixelsPerFrame);
                mValueInput.setDisplayedValue(dFrame);
            }
            const int dDFrame = dFrame - mMoveDFrame;

            if(mMovingKeys) {
                if(mFirstMove) {
                    if(mGraphViewed) {
                        for(const auto& anim : mGraphAnimators) {
                            anim->graph_startSelectedKeysTransform();
                        }
                    } else {
                        for(const auto& anim : mSelectedKeysAnimators) {
                            anim->startSelectedKeysTransform();
                        }
                    }
                }
                if(mScalingKeys) {
                    qreal keysScale;
                    if(mValueInput.inputEnabled()) {
                        keysScale = mValueInput.getValue();
                    } else {
                        keysScale = 1 + (posU.x() - mLastPressPos.x())/150.;
                        mValueInput.setDisplayedValue(keysScale);
                    }
                    const int absFrame = mMainWindow->getCurrentFrame();
                    if(mGraphViewed) {
                        for(const auto& anim : mGraphAnimators) {
                            anim->scaleSelectedKeysFrame(absFrame, keysScale);
                        }
                    } else {
                        for(const auto& anim : mSelectedKeysAnimators) {
                            anim->scaleSelectedKeysFrame(absFrame, keysScale);
                        }
                    }
                } else {
                    mMoveDFrame = dFrame;
                    if(mGraphViewed) {
                        const int dY = mLastPressPos.y() - posU.y();
                        const qreal dValue = mValueInput.xOnlyMode() ? 0 :
                                    dY/mPixelsPerValUnit;
                        const qreal dFrameV = mValueInput.yOnlyMode() ?
                                    0 : qreal(dFrame);
                        for(const auto& anim : mGraphAnimators) {
                            anim->graph_changeSelectedKeysFrameAndValue(
                                            {dFrameV, dValue});
                        }
                    } else if(dDFrame != 0) {
                        for(const auto& anim : mSelectedKeysAnimators) {
                            anim->incSelectedKeysFrame(dDFrame);
                        }
                    }
                }
            } else if(mMovingRect) {
                auto canvasWindow = mMainWindow->getCanvasWindow();
                if(mFirstMove) {
                    if(mLastPressedMovable) {
                        if(!mLastPressedMovable->isSelected()) {
                            mLastPressedMovable->pressed(
                                        mMainWindow->isShiftPressed());
                        }

                        const auto childProp = mLastPressedMovable->getChildProperty();
                        if(childProp->SWT_isBoundingBox()) {
                            if(mLastPressedMovable->isDurationRect()) {
                                canvasWindow->startDurationRectPosTransformForAllSelected();
                            } else if(mLastPressedMovable->isMaxFrame()) {
                                canvasWindow->startMaxFramePosTransformForAllSelected();
                            } else if(mLastPressedMovable->isMinFrame()) {
                                canvasWindow->startMinFramePosTransformForAllSelected();
                            }
                        } else {
                            mLastPressedMovable->startPosTransform();
                        }
                    }
                }

                if(dDFrame != 0) {
                    mMoveDFrame = dFrame;
                    if(mLastPressedMovable) {
                        const auto childProp = mLastPressedMovable->getChildProperty();
                        if(childProp->SWT_isBoundingBox()) {
                            if(mLastPressedMovable->isDurationRect()) {
                                canvasWindow->moveDurationRectForAllSelected(dDFrame);
                            } else if(mLastPressedMovable->isMaxFrame()) {
                                canvasWindow->moveMaxFrameForAllSelected(dDFrame);
                            } else if(mLastPressedMovable->isMinFrame()) {
                                canvasWindow->moveMinFrameForAllSelected(dDFrame);
                            }
                        } else {
                            mLastPressedMovable->changeFramePosBy(dDFrame);
                        }
                    }
//                        mLastPressedDurationRectangleMovable->changeFramePosBy(
//                                    dDFrame);
                }
            } else if(mSelecting) {
                if(mGraphViewed) {
                    qreal value;
                    qreal frame;
                    graphGetValueAndFrameFromPos(posU, &value, &frame);
                    mSelectionRect.setBottomRight(QPointF(frame, value));
                } else {
                    const qreal posUXFrame = posU.x()/mPixelsPerFrame +
                            mMinViewedFrame;
                    mSelectionRect.setBottomRight(
                                    QPointF(posUXFrame, posU.y() + mViewedTop));
                }
            }
            mFirstMove = false;
        }
    } else {
        updateHoveredPointFromPos(posU);
    }

    mMainWindow->queScheduledTasksAndUpdate();
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
    if(mGraphViewed && mPressedPoint) {
        if(mPressedCtrlPoint) {
            mPressedPoint->setSelected(false);
            mPressedCtrlPoint = false;
        } else {
            if(mFirstMove) {
                if(!mMainWindow->isShiftPressed()) {
                    clearKeySelection();
                    addKeyToSelection(mPressedPoint->getParentKey());
                }
            } else {
                for(const auto& anim : mGraphAnimators) {
                    if(!anim->hasSelectedKeys()) continue;
                    anim->graph_finishSelectedKeysTransform();
                }
            }
        }
        mPressedPoint = nullptr;

        graphConstrainAnimatorCtrlsFrameValues();
        //graphConstrainAnimatorCtrlsFrameValues();

        // needed ?
        graphUpdateDimensions();
    } else {
        if(e->button() == Qt::LeftButton) {
            if(mSelecting) {
                if(mSelectionRect.left() > mSelectionRect.right()) {
                    const qreal rightT = mSelectionRect.right();
                    const qreal leftT = mSelectionRect.left();
                    mSelectionRect.setLeft(rightT);
                    mSelectionRect.setRight(leftT);
                }
                if(mSelectionRect.top() > mSelectionRect.bottom()) {
                    const qreal bottomT = mSelectionRect.bottom();
                    const qreal topT = mSelectionRect.top();
                    mSelectionRect.setTop(bottomT);
                    mSelectionRect.setBottom(topT);
                }
                if(!mMainWindow->isShiftPressed()) {
                    clearKeySelection();
                }
                selectKeysInSelectionRect();

                mSelecting = false;
            } else if(mMovingKeys) {
                if(mFirstMove) {
                    if(!mMainWindow->isShiftPressed()) {
                        clearKeySelection();
                        addKeyToSelection(mLastPressedKey);
                    }
                }
                for(const auto& anim : mSelectedKeysAnimators) {
                    anim->finishSelectedKeysTransform();
                }

                mMoveDFrame = 0;
                mMovingKeys = false;
                mScalingKeys = false;
            } else if(mMovingRect) {
                if(mFirstMove) {
                    if(mLastPressedMovable) {
                        mLastPressedMovable->pressed(
                                    mMainWindow->isShiftPressed());
                    }
                } else {
                    mMoveDFrame = 0;
                    mMovingRect = false;
                    auto canvasWindow = mMainWindow->getCanvasWindow();
                    const auto childProp = mLastPressedMovable->getChildProperty();
                    if(childProp->SWT_isBoundingBox()) {
                        if(mLastPressedMovable->isDurationRect()) {
                            canvasWindow->finishDurationRectPosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMinFrame()) {
                            canvasWindow->finishMinFramePosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMaxFrame()) {
                            canvasWindow->finishMaxFramePosTransformForAllSelected();
                        }
                    } else {
                        mLastPressedMovable->finishPosTransform();
                    }
                }
            }
        }
    }
    updateHoveredPointFromPos(e->pos() + QPoint(-MIN_WIDGET_HEIGHT/2, 0));
    releaseMouseAndDontTrack();

    mValueInput.clearAndDisableInput();
    mMainWindow->queScheduledTasksAndUpdate();
}

void KeysView::setFramesRange(const int &startFrame,
                              const int &endFrame) {
    mMinViewedFrame = startFrame;
    mMaxViewedFrame = endFrame;
    updatePixelsPerFrame();
    if(mGraphViewed) graphUpdateDimensions();
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
    const qreal animWidth = width() - 2*MIN_WIDGET_HEIGHT;
    const qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

void KeysView::addKeyToSelection(Key * const key) {
    QList<qptr<Animator>> toSelect;
    key->addToSelection(toSelect);
    for(const auto& anim : toSelect) {
        connect(anim, &QObject::destroyed, this, [this, anim]() {
            mSelectedKeysAnimators.removeOne(anim);
        });
        mSelectedKeysAnimators << anim;
    }
}

void KeysView::removeKeyFromSelection(Key * const key) {
    QList<qptr<Animator>> toRemove;
    key->removeFromSelection(toRemove);
    for(const auto& anim : toRemove) {
        disconnect(anim, &QObject::destroyed, this, nullptr);
        mSelectedKeysAnimators.removeOne(anim);
    }
}

void KeysView::clearKeySelection() {
    for(const auto& anim : mSelectedKeysAnimators) {
        anim->deselectAllKeys();
    }
    mSelectedKeysAnimators.clear();
}
