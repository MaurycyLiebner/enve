#include "canvas.h"
#include <QMouseEvent>
#include "pathpivot.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/imagebox.h"
#include "Boxes/textbox.h"

QPointF Canvas::scaleDistancePointByCurrentScale(QPointF point) {
    return point/mCombinedTransformMatrix.m11();
}

void Canvas::handleMovePathMousePressEvent() {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPos);
    if(mLastPressedBox == NULL) {
        if(!isShiftPressed() ) {
            mCurrentBoxesGroup->clearBoxesSelection();
        }
        mSelecting = true;
        startSelectionAtPoint(mLastMouseEventPos);
    } else {
        if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
            mCurrentBoxesGroup->clearBoxesSelection();
        }
    }
}

#include <QMenu>
void Canvas::handleRightButtonMousePress(QMouseEvent *event) {
    if(mIsMouseGrabbing) {
        cancelCurrentTransform();
        clearAndDisableInput();
    } else {
        BoundingBox *pressedBox = mCurrentBoxesGroup->getBoxAt(
                                                        event->pos());
        if(pressedBox == NULL) {
            mCurrentBoxesGroup->clearBoxesSelection();

            QMenu menu(mCanvasWidget);

            menu.addAction("Paste");
            QMenu *linkCanvasMenu = menu.addMenu("Link Canvas");
            const QList<Canvas*> &listOfCanvas = mCanvasWidget->getCanvasList();
            foreach(Canvas *canvas, listOfCanvas) {
                QAction *action = linkCanvasMenu->addAction(canvas->getName());
                if(canvas == this) {
                    action->setEnabled(false);
                    action->setVisible(false);
                }
            }

            QMenu *effectsMenu = menu.addMenu("Effects");
            effectsMenu->addAction("Blur");
//            effectsMenu->addAction("Brush");
            effectsMenu->addAction("Lines");
            effectsMenu->addAction("Circles");
            effectsMenu->addAction("Swirl");
            effectsMenu->addAction("Oil");
            effectsMenu->addAction("Implode");
            effectsMenu->addAction("Desaturate");

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction != NULL) {
                if(selectedAction->text() == "Paste") {
                } else if(selectedAction->text() == "Blur") {
                    addEffect(new BlurEffect());
                } /*else if(selectedAction->text() == "Brush") {
                    addEffect(new BrushEffect());
                }*/ else if(selectedAction->text() == "Lines") {
                    addEffect(new LinesEffect());
                } else if(selectedAction->text() == "Circles") {
                    addEffect(new CirclesEffect());
                } else if(selectedAction->text() == "Swirl") {
                    addEffect(new SwirlEffect());
                } else if(selectedAction->text() == "Oil") {
                    addEffect(new OilEffect());
                } else if(selectedAction->text() == "Implode") {
                    addEffect(new ImplodeEffect());
                } else if(selectedAction->text() == "Desaturate") {
                    addEffect(new DesaturateEffect());
                } else { // link canvas
                    const QList<QAction*> &canvasActions =
                            linkCanvasMenu->actions();
                    int id = canvasActions.indexOf(selectedAction);
                    if(id >= 0) {
                        listOfCanvas.at(id)->createLink(
                                    mCurrentBoxesGroup)->centerPivotPosition();
                    }
                }
            } else {

            }
        } else {
            if(!pressedBox->isSelected()) {
                mCurrentBoxesGroup->clearBoxesSelection();
                mCurrentBoxesGroup->addBoxToSelection(pressedBox);
            }

            QMenu menu(mCanvasWidget);

            menu.addAction("Apply Transformation");
            menu.addAction("Create Link");
            menu.addAction("Center Pivot");
            menu.addAction("Copy");
            menu.addAction("Cut");
            menu.addAction("Duplicate");
            menu.addAction("Group");
            menu.addAction("Ungroup");
            menu.addAction("Delete");

            QMenu *effectsMenu = menu.addMenu("Effects");
            effectsMenu->addAction("Blur");
            effectsMenu->addAction("Shadow");
//            effectsMenu->addAction("Brush");
            effectsMenu->addAction("Lines");
            effectsMenu->addAction("Circles");
            effectsMenu->addAction("Swirl");
            effectsMenu->addAction("Oil");
            effectsMenu->addAction("Implode");
            effectsMenu->addAction("Desaturate");

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction != NULL) {
                if(selectedAction->text() == "Delete") {
                    mCurrentBoxesGroup->removeSelectedBoxesAndClearList();
                } else if(selectedAction->text() == "Apply Transformation") {
                    mCurrentBoxesGroup->applyCurrentTransformationToSelected();
                } else if(selectedAction->text() == "Create Link") {
                    mCurrentBoxesGroup->createLinkBoxForSelected();
                } else if(selectedAction->text() == "Group") {
                    groupSelectedBoxesAction();
                } else if(selectedAction->text() == "Ungroup") {
                    mCurrentBoxesGroup->ungroupSelected();
                } else if(selectedAction->text() == "Center Pivot") {
                    mCurrentBoxesGroup->centerPivotForSelected();
                } else if(selectedAction->text() == "Blur") {
                    mCurrentBoxesGroup->applyBlurToSelected();
                } else if(selectedAction->text() == "Shadow") {
                    mCurrentBoxesGroup->applyShadowToSelected();
                } else if(selectedAction->text() == "Brush") {
                    mCurrentBoxesGroup->applyBrushEffectToSelected();
                } else if(selectedAction->text() == "Lines") {
                    mCurrentBoxesGroup->applyLinesEffectToSelected();
                } else if(selectedAction->text() == "Circles") {
                    mCurrentBoxesGroup->applyCirclesEffectToSelected();
                } else if(selectedAction->text() == "Swirl") {
                    mCurrentBoxesGroup->applySwirlEffectToSelected();
                } else if(selectedAction->text() == "Oil") {
                    mCurrentBoxesGroup->applyOilEffectToSelected();
                } else if(selectedAction->text() == "Implode") {
                    mCurrentBoxesGroup->applyImplodeEffectToSelected();
                } else if(selectedAction->text() == "Desaturate") {
                    mCurrentBoxesGroup->applyDesaturateEffectToSelected();
                }
            } else {

            }
        }
    }
}
#include "Boxes/particlebox.h"
void Canvas::handleLeftButtonMousePress(QMouseEvent *event) {
    if(mIsMouseGrabbing) {
        //handleMouseRelease(event->pos());
        //releaseMouseAndDontTrack();
        return;
    }

    grabMouseAndTrack();

    mDoubleClick = false;
    mMovesToSkip = 2;
    mFirstMouseMove = true;

    mLastPressPos = mLastMouseEventPos;

    if(mRotPivot->handleMousePress(mLastMouseEventPos)) {
    } else if(isMovingPath()) {
        handleMovePathMousePressEvent();
    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mLastPressedBox = getPathAtFromAllAncestors(mLastPressPos);
    } else {
        mLastPressedPoint = mCurrentBoxesGroup->getPointAt(mLastMouseEventPos, mCurrentMode);


        if(mCurrentMode == CanvasMode::ADD_POINT) {
            if(mCurrentEndPoint != NULL) {
                if(mCurrentEndPoint->isHidden()) {
                    setCurrentEndPoint(NULL);
                }
            }
            PathPoint *pathPointUnderMouse = (PathPoint*) mLastPressedPoint;
            if( (pathPointUnderMouse == NULL) ? false :
                    !pathPointUnderMouse->isEndPoint() ) {
                pathPointUnderMouse = NULL;
            }
            if(pathPointUnderMouse == mCurrentEndPoint && pathPointUnderMouse != NULL) {
                return;
            }
            if(mCurrentEndPoint == NULL && pathPointUnderMouse == NULL) {

                VectorPath *newPath = new VectorPath(mCurrentBoxesGroup);
                mCurrentBoxesGroup->clearBoxesSelection();
                mCurrentBoxesGroup->addBoxToSelection(newPath);
                setCurrentEndPoint(
                            newPath->addPointAbsPos(mLastMouseEventPos,
                                                    mCurrentEndPoint) );

            } else {
                if(pathPointUnderMouse == NULL) {
                    setCurrentEndPoint(mCurrentEndPoint->addPointAbsPos(mLastMouseEventPos) );
                } else if(mCurrentEndPoint == NULL) {
                    setCurrentEndPoint(pathPointUnderMouse);
                } else {
                    if(mCurrentEndPoint->getParentPath() ==
                       pathPointUnderMouse->getParentPath()) {
                        mCurrentEndPoint->getParentPath()->
                                connectPoints(mCurrentEndPoint, pathPointUnderMouse);
                    }
                    else {
                        connectPointsFromDifferentPaths(mCurrentEndPoint,
                                                        pathPointUnderMouse);
                    }
                    setCurrentEndPoint(pathPointUnderMouse);
                }
            } // pats is not null
        } // point adding mode
        else if (mCurrentMode == CanvasMode::MOVE_POINT) {
            if (mLastPressedPoint == NULL) {
                if(isCtrlPressed() ) {
                    mCurrentBoxesGroup->clearPointsSelection();
                    mLastPressedPoint = mCurrentBoxesGroup->
                            createNewPointOnLineNearSelected(mLastPressPos,
                                                             isShiftPressed());
                } else {
                    mCurrentEdge = mCurrentBoxesGroup->getPressedEdge(
                                                            mLastPressPos);
                    if(mCurrentEdge == NULL) {
                        mSelecting = true;
                        startSelectionAtPoint(mLastMouseEventPos);
                    } else {
                        mCurrentBoxesGroup->clearPointsSelection();
                    }
                }
            } else {
                if(mLastPressedPoint->isSelected()) {
                    return;
                }
                if(!isShiftPressed() &&
                        !(mLastPressedPoint->isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible()) ) {
                    mCurrentBoxesGroup->clearPointsSelection();
                }
                if(mLastPressedPoint->isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible() ) {
                    mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
                }
            }
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {

            Circle *newPath = new Circle(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPos, false);
            newPath->startAllPointsTransform();
            mCurrentBoxesGroup->clearBoxesSelection();
            mCurrentBoxesGroup->addBoxToSelection(newPath);

            mCurrentCircle = newPath;

        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            ParticleBox *newBox = new ParticleBox(mCurrentBoxesGroup);

            Rectangle *newPath = new Rectangle(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPos, false);
            newPath->startAllPointsTransform();
            mCurrentBoxesGroup->clearBoxesSelection();
            mCurrentBoxesGroup->addBoxToSelection(newPath);

            mCurrentRectangle = newPath;

        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {

            TextBox *newPath = new TextBox(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPos, false);

            mCurrentTextBox = newPath;

            mCurrentBoxesGroup->clearBoxesSelection();
            mCurrentBoxesGroup->addBoxToSelection(newPath);

        }
    } // current mode allows interaction with points
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(mPreviewing) return;
    mLastMouseEventPos = event->pos();
    if(event->button() != Qt::LeftButton) {
        if(event->button() == Qt::RightButton) {
            handleRightButtonMousePress(event);
        }
    } else {
        handleLeftButtonMousePress(event);
    }

    callUpdateSchedulers();
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        mCurrentBoxesGroup->cancelSelectedPointsTransform();
        if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
            mRotPivot->handleMouseRelease();
        }
    } else if(mCurrentMode == CanvasMode::MOVE_PATH) {
        if(mRotPivot->isSelected()) {
            mRotPivot->cancelTransform();
        } else {
            mCurrentBoxesGroup->cancelSelectedBoxesTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {

    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        setCanvasMode(MOVE_PATH);
    }

    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
}

void Canvas::handleMovePointMouseRelease(QPointF pos) {
    if(mRotPivot->isSelected()) {
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        mCurrentBoxesGroup->finishSelectedPointsTransform();
    } else if(mSelecting) {
        mSelecting = false;
        if(mFirstMouseMove) {
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(pos);
            if((mLastPressedBox == NULL) ? true : mLastPressedBox->isGroup()) {
                BoundingBox *pressedBox = getPathAtFromAllAncestors(pos);
                if(pressedBox == NULL) {
                    if(!(isShiftPressed()) ) {
                        mCurrentBoxesGroup->clearPointsSelectionOrDeselect();
                    }
                } else {
                    mCurrentBoxesGroup->clearPointsSelection();
                    setCurrentBoxesGroup((BoxesGroup*) pressedBox->getParent());
                    mCurrentBoxesGroup->addBoxToSelection(pressedBox);
                }
            }
            if(mLastPressedBox != NULL) {
                if(isShiftPressed()) {
                    if(mLastPressedBox->isSelected()) {
                        mCurrentBoxesGroup->removeBoxFromSelection(mLastPressedBox);
                    } else {
                        mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
                    }
                } else {
                    mCurrentBoxesGroup->clearPointsSelection();
                    selectOnlyLastPressedBox();
                }
            }
            return;
        }
        if(!isShiftPressed()) mCurrentBoxesGroup->clearPointsSelection();
        moveSecondSelectionPoint(pos);
        mCurrentBoxesGroup->selectAndAddContainedPointsToSelection(mSelectionRect);
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isSelected()) {
                    mCurrentBoxesGroup->removePointFromSelection(mLastPressedPoint);
                } else {
                    mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
                }
            }
        } else {
            selectOnlyLastPressedPoint();
        }
    } else {
        mCurrentBoxesGroup->finishSelectedPointsTransform();
        if(mLastPressedPoint != NULL) {
            if(mLastPressedPoint->isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible() ) {
                mCurrentBoxesGroup->removePointFromSelection(mLastPressedPoint);
            }
        }
    }
}


void Canvas::handleMovePathMouseRelease(QPointF pos) {
    if(mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        mCurrentBoxesGroup->finishSelectedBoxesTransform();
    } else if(mFirstMouseMove) {
        mSelecting = false;
        if(isShiftPressed() && mLastPressedBox != NULL) {
            if(mLastPressedBox->isSelected()) {
                mCurrentBoxesGroup->removeBoxFromSelection(mLastPressedBox);
            } else {
                mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
            }
        } else {
            selectOnlyLastPressedBox();
        }
    } else if(mSelecting) {
        moveSecondSelectionPoint(pos);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        mCurrentBoxesGroup->finishSelectedBoxesTransform();
    }
}

void Canvas::handleAddPointMouseRelease() {
    if(mCurrentEndPoint != NULL) {
        if(!mCurrentEndPoint->isEndPoint()) {
            setCurrentEndPoint(NULL);
        }
    }
}

void Canvas::handleMouseRelease(QPointF eventPos) {
    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
    if(!mDoubleClick) {
        if(mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMouseRelease(eventPos);
        } else if(isMovingPath()) {
            handleMovePathMouseRelease(eventPos);
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseRelease();
        } else if(mCurrentMode == PICK_PATH_SETTINGS) {
            if(mLastPressedBox != NULL) {
                mFillStrokeSettingsWidget->loadSettingsFromPath(
                            (VectorPath*) mLastPressedBox);
            }
            mCanvasWidget->setCanvasMode(MOVE_PATH);
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
            if(mCurrentTextBox != NULL) {
                mCurrentTextBox->openTextEditor();
            }
        }
    }

    mLastPressedBox = NULL;
    mLastPressedPoint = NULL;
    if(mCurrentEdge != NULL) {
        if(!mFirstMouseMove) {
            mCurrentEdge->finishTransform();
        }
        delete mCurrentEdge;
        mCurrentEdge = NULL;
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(mPreviewing) return;
    mXOnlyTransform = false;
    mYOnlyTransform = false;
    if(mInputTransformationEnabled) {
        mFirstMouseMove = false;
    }
    clearAndDisableInput();
    if(mTransformationFinishedBeforeMouseRelease) {
        mTransformationFinishedBeforeMouseRelease = false;
    } else {
        if(event->button() != Qt::LeftButton) {
            if(mIsMouseGrabbing) {
                releaseMouseAndDontTrack();
            }
        }
        handleMouseRelease(event->pos());
    }
    callUpdateSchedulers();
}

QPointF Canvas::getMoveByValueForEventPos(QPointF eventPos) {
    QPointF moveByPoint = eventPos - mLastPressPos;
    if(mInputTransformationEnabled) {
        moveByPoint = QPointF(mInputTransformationValue,
                              mInputTransformationValue);
    }
    if(mYOnlyTransform) {
        moveByPoint.setX(0.);
    } else if(mXOnlyTransform) {
        moveByPoint.setY(0.);
    }
    return moveByPoint;
}

void Canvas::handleMovePointMouseMove(QPointF eventPos) {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }
        mRotPivot->moveByAbs(getMoveByValueForEventPos(eventPos));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
           mRotPivot->handleMouseMove(eventPos, mLastPressPos,
                                      mXOnlyTransform, mYOnlyTransform,
                                      mInputTransformationEnabled,
                                      mInputTransformationValue,
                                      mFirstMouseMove,
                                      mCurrentMode);
    } else if(mCurrentEdge != NULL) {
        if(mFirstMouseMove) {
            mCurrentEdge->startTransform();
        }
        mCurrentEdge->makePassThrough(eventPos);
    } else {
        if(mLastPressedPoint != NULL) {
            mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);

            if(mLastPressedPoint->isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible() ) {
                if(mFirstMouseMove) {
                    mCurrentBoxesGroup->startSelectedPointsTransform();
                }
                mLastPressedPoint->moveByAbs(getMoveByValueForEventPos(eventPos) );
                return;//
            }/* else {
                mCurrentBoxesGroup->moveSelectedPointsBy(getMoveByValueForEventPos(eventPos),
                                                         mFirstMouseMove);
            }*/
        }
        mCurrentBoxesGroup->moveSelectedPointsBy(getMoveByValueForEventPos(eventPos),
                                                 mFirstMouseMove);
    }
}

void Canvas::setPivotPositionForSelected() {
    mCurrentBoxesGroup->setSelectedPivotAbsPos(mRotPivot->getAbsolutePos());
}

void Canvas::handleMovePathMouseMove(QPointF eventPos) {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }

        mRotPivot->moveByAbs(getMoveByValueForEventPos(eventPos));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseMove(eventPos, mLastPressPos,
                                   mXOnlyTransform, mYOnlyTransform,
                                   mInputTransformationEnabled,
                                   mInputTransformationValue,
                                   mFirstMouseMove, mCurrentMode);
    } else {
        if(mLastPressedBox != NULL) {
            mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
        }

        mCurrentBoxesGroup->moveSelectedBoxesBy(getMoveByValueForEventPos(eventPos),
                    mFirstMouseMove);
    }
}

void Canvas::handleAddPointMouseMove(QPointF eventPos) {
    if(mCurrentEndPoint == NULL) return;
    mCurrentEndPoint->setCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
    mCurrentEndPoint->moveEndCtrlPtToAbsPos(eventPos);
}

void Canvas::updateTransformation() {
    QPointF eventPos = mLastMouseEventPos;

    if(mSelecting) {
        moveSecondSelectionPoint(eventPos);
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseMove(eventPos);
    } else if(isMovingPath()) {
        handleMovePathMouseMove(eventPos);
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddPointMouseMove(eventPos);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(mPreviewing) return;
    if(mMovesToSkip > 0) {
        mMovesToSkip--;
        return;
    }
    QPointF eventPos = event->pos();

    if(event->buttons() & Qt::MiddleButton) {
        moveBy(event->pos() - mLastMouseEventPos);
    } else if(!mTransformationFinishedBeforeMouseRelease) {
        if(mSelecting) {
            moveSecondSelectionPoint(eventPos);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMouseMove(eventPos);
        } else if(isMovingPath()) {
            handleMovePathMouseMove(eventPos);
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove(eventPos);
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
            if(isShiftPressed() ) {
                qreal lenR = pointToLen(eventPos - mLastPressPos);
                mCurrentCircle->moveRadiusesByAbs(QPointF(lenR, lenR));
            } else {
                mCurrentCircle->moveRadiusesByAbs(eventPos - mLastPressPos);
            }
        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            if(isShiftPressed()) {
                QPointF trans = eventPos - mLastPressPos;
                qreal valF = qMax(trans.x(), trans.y() );
                trans = QPointF(valF, valF);
                mCurrentRectangle->moveSizePointByAbs(trans);
            } else {
                mCurrentRectangle->moveSizePointByAbs(eventPos - mLastPressPos);
            }
        }
    }
    mLastMouseEventPos = event->pos();
    mFirstMouseMove = false;

    callUpdateSchedulers();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    if(mPreviewing) return;
    if(event->delta() > 0) {
        scale(1.2, event->posF());
    } else {
        scale(0.8, event->posF());
    }
    mVisibleHeight = mCombinedTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCombinedTransformMatrix.m11()*mWidth;
    
    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    mDoubleClick = true;

    mLastPressedPoint = mCurrentBoxesGroup->
            createNewPointOnLineNearSelected(mLastPressPos, true);

    if(mLastPressedPoint == NULL) {
        BoundingBox *boxAt = mCurrentBoxesGroup->getBoxAt(event->pos());
        if(boxAt == NULL) {
            if(mCurrentBoxesGroup != this) {
                setCurrentBoxesGroup((BoxesGroup*) mCurrentBoxesGroup->getParent());
            }
        } else {
            if(boxAt->isGroup()) {
                setCurrentBoxesGroup((BoxesGroup*) boxAt);
            } else if(boxAt->isText()) {
                ((TextBox*) boxAt)->openTextEditor();
            } else if(boxAt->isCircle() ) {
                ((Circle*) boxAt)->objectToPath();
            } else if(mCurrentMode == MOVE_PATH) {
                setCanvasMode(MOVE_PATH);
            }
        }
    }
    
    callUpdateSchedulers();
}
