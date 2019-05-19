#include "canvas.h"
#include <QMouseEvent>
#include <QMenu>
#include "MovablePoints/pathpivot.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/imagebox.h"
#include "Boxes/textbox.h"
#include "Boxes/linkbox.h"
#include "pointhelpers.h"
#include "Boxes/particlebox.h"
#include "clipboardcontainer.h"
#include "GUI/mainwindow.h"
#include "Boxes/paintbox.h"
#include "GUI/fontswidget.h"
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
#include <QFileDialog>
#include "GUI/paintboxsettingsdialog.h"
#include "GUI/customfpsdialog.h"
#include "GPUEffects/gpurastereffect.h"
#include "MovablePoints/smartnodepoint.h"
#include "pointtypemenu.h"
#include "Boxes/containerbox.h"

void Canvas::handleMovePathMousePressEvent() {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPosRel);
    if(!mLastPressedBox) {
        if(!isShiftPressed()) clearBoxesSelection();
    } else {
        if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
            clearBoxesSelection();
        }
    }
}

void Canvas::addSelectedBoxesActions(QMenu * const qMenu) {
    qMenu->addSeparator();
    qMenu->addAction("Create Link", [this]() {
        for(const auto& box : mSelectedBoxes)
            mCurrentBoxesGroup->addContainedBox(box->createLink());
    });
    qMenu->addAction("Center Pivot", [this]() {
        centerPivotForSelected();
    });
    qMenu->addSeparator();

    QAction * const copyAction = qMenu->addAction("Copy", [this]() {
        this->copyAction();
    });
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);

    QAction * const cutAction = qMenu->addAction("Cut", [this]() {
        this->cutAction();
    });
    cutAction->setShortcut(Qt::CTRL + Qt::Key_X);

    QAction * const duplicateAction = qMenu->addAction("Duplicate", [this]() {
        this->duplicateSelectedBoxes();
    });
    duplicateAction->setShortcut(Qt::CTRL + Qt::Key_D);

    QAction * const deleteAction = qMenu->addAction("Delete", [this]() {
        this->removeSelectedBoxesAndClearList();
    });
    deleteAction->setShortcut(Qt::Key_Delete);

    qMenu->addSeparator();

    QAction * const groupAction = qMenu->addAction("Group", [this]() {
        this->groupSelectedBoxes();
    });
    groupAction->setShortcut(Qt::CTRL + Qt::Key_G);

    BoxTypeMenu menu(qMenu, this, mMainWindow);
    for(const auto& box : mSelectedBoxes) {
        if(menu.hasActionsForType(box)) continue;
        box->addActionsToMenu(&menu);
        menu.addedActionsForType(box);
    }
}

#include <QInputDialog>
#include "PathEffects/patheffect.h"
void Canvas::addActionsToMenu(QMenu * const menu) {
    const BoxesClipboardContainer * const clipboard =
            MainWindow::getBoxesClipboardContainer();
    if(clipboard) {
        QAction * const pasteAct = menu->addAction("Paste", this,
                                                  &Canvas::pasteAction);
        pasteAct->setShortcut(Qt::CTRL + Qt::Key_V);
    }

    const auto &listOfCanvas = mCanvasWindow->getCanvasList();
    if(listOfCanvas.count() > 1) {
        QMenu * const linkCanvasMenu = menu->addMenu("Link Canvas");
        for(const auto& canvas : listOfCanvas) {
            const auto slot = [this, canvas]() {
                auto newLink = canvas->createLink();
                mCurrentBoxesGroup->addContainedBox(newLink);
                newLink->centerPivotPosition();
            };
            QAction * const action = linkCanvasMenu->addAction(
                        canvas->getName(), this, slot);
            if(canvas == this) {
                action->setEnabled(false);
                action->setVisible(false);
            }
        }
    }

    QMenu * const effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur", [this]() {
        addEffect<BlurEffect>();
    });

    effectsMenu->addAction("Motion Blur", [this]() {
        addEffect<SampledMotionBlurEffect>();
    });

    effectsMenu->addAction("Shadow", [this]() {
        addEffect<ShadowEffect>();
    });

    effectsMenu->addAction("Desaturate", [this]() {
        addEffect<DesaturateEffect>();
    });

    effectsMenu->addAction("Colorize", [this]() {
        addEffect<ColorizeEffect>();
    });

    effectsMenu->addAction("Contrast", [this]() {
        addEffect<ContrastEffect>();
    });

    effectsMenu->addAction("Brightness", [this]() {
        addEffect<BrightnessEffect>();
    });

    effectsMenu->addAction("Replace Color", [this]() {
        addEffect<ReplaceColorEffect>();
    });

    QMenu * const pathEffectsMenu = menu->addMenu("Path Effects");
    pathEffectsMenu->addAction("Displace Effect", [this]() {
        addPathEffect(SPtrCreate(DisplacePathEffect)(false));
    });

    pathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addPathEffect(SPtrCreate(DuplicatePathEffect)(false));
    });

    pathEffectsMenu->addAction("Sub-Path Effect", [this]() {
        addPathEffect(SPtrCreate(SubPathEffect)(false));
    });

    pathEffectsMenu->addAction("Solidify Effect", [this]() {
        addPathEffect(SPtrCreate(SolidifyPathEffect)(false));
    });

    pathEffectsMenu->addAction("Sum Effect", [this]() {
        addPathEffect(SPtrCreate(SumPathEffect)(false));
    });
//    pathEffectsMenu->addAction("Operation Effect");
//    pathEffectsMenu->addAction("Group Sum Effect");

    QMenu * const fillPathEffectsMenu = menu->addMenu("Fill Effects");

    fillPathEffectsMenu->addAction("Displace Effect", [this]() {
        addFillPathEffect(SPtrCreate(DisplacePathEffect)(false));
    });

    fillPathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addFillPathEffect(SPtrCreate(DuplicatePathEffect)(false));
    });

    fillPathEffectsMenu->addAction("Solidify Effect", [this]() {
        addFillPathEffect(SPtrCreate(SolidifyPathEffect)(false));
    });

    fillPathEffectsMenu->addAction("Sum Effect", [this]() {
        addFillPathEffect(SPtrCreate(SumPathEffect)(false));
    });
//    fillPathEffectsMenu->addAction("Operation Effect");

    QMenu * const outlinePathEffectsMenu = menu->addMenu("Outline Effects");

    outlinePathEffectsMenu->addAction("Displace Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DisplacePathEffect)(true));
    });

    outlinePathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DuplicatePathEffect)(true));
    });

    pathEffectsMenu->addAction("Sub-Path Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SubPathEffect)(true));
    });

    outlinePathEffectsMenu->addAction("Solidify Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SolidifyPathEffect)(true));
    });

    outlinePathEffectsMenu->addAction("Sum Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SumPathEffect)(true));
    });
//    outlinePathEffectsMenu->addAction("Operation Effect");

    menu->addAction("Map to Different Fps...", [this]() {
        bool ok;
        const qreal newFps = QInputDialog::getDouble(
                    mMainWindow, "Map to Different Fps",
                    "New Fps:", mFps, 1, 999, 2, &ok);
        if(ok) changeFpsTo(newFps);
    });

    menu->addAction("Settings...", [this]() {
        mCanvasWindow->openSettingsWindowForCurrentCanvas();
    });
}

void Canvas::handleRightButtonMousePress(const QMouseEvent * const event) {
    if(mIsMouseGrabbing) {
        cancelCurrentTransform();
        mValueInput.clearAndDisableInput();
    } else {
        mLastPressedBox = mHoveredBox;
        mLastPressedPoint = mHoveredPoint_d;
        if(mLastPressedPoint) {
            QMenu qMenu;
            PointTypeMenu menu(&qMenu, this, mMainWindow);
            if(mLastPressedPoint->selectionEnabled()) {
                if(!mLastPressedPoint->isSelected()) {
                    if(!isShiftPressed()) clearPointsSelection();
                    addPointToSelection(mLastPressedPoint);
                }
                for(const auto& pt : mSelectedPoints_d) {
                    if(menu.hasActionsForType(pt)) continue;
                    pt->canvasContextMenu(&menu);
                    menu.addedActionsForType(pt);
                }
            } else {
                mLastPressedPoint->canvasContextMenu(&menu);
            }
            qMenu.exec(event->globalPos());
        } else if(mLastPressedBox) {
            if(!mLastPressedBox->isSelected()) {
                if(!isShiftPressed()) clearBoxesSelection();
                addBoxToSelection(mLastPressedBox);
            }

            QMenu qMenu;
            addSelectedBoxesActions(&qMenu);
            qMenu.exec(event->globalPos());
        } else {
            clearPointsSelection();
            clearBoxesSelection();
            QMenu menu(mCanvasWindow->getCanvasWidget());
            addActionsToMenu(&menu);
            menu.exec(event->globalPos());
        }
    }
}

void Canvas::clearHoveredEdge() {
    mHoveredNormalSegment.reset();
}

void Canvas::handleMovePointMousePressEvent() {
    if(mHoveredNormalSegment.isValid()) {
        if(isCtrlPressed()) {
            clearPointsSelection();
            mLastPressedPoint = mHoveredNormalSegment.divideAtAbsPos(
                                        mLastPressPosRel);
        } else {
            mCurrentNormalSegment = mHoveredNormalSegment;
            mCurrentNormalSegmentT = mCurrentNormalSegment.closestAbsT(
                        mLastPressPosRel);
            clearPointsSelection();
            clearCurrentSmartEndPoint();
            clearLastPressedPoint();
        }
        clearHoveredEdge();
    } else if(mLastPressedPoint) {
        if(mLastPressedPoint->isSelected()) return;
        if(!isShiftPressed() && mLastPressedPoint->selectionEnabled()) {
            clearPointsSelection();
        }
        if(!mLastPressedPoint->selectionEnabled()) {
            addPointToSelection(mLastPressedPoint);
        }
    }
}


void Canvas::handleLeftButtonMousePress() {
    if(mIsMouseGrabbing) {
        //handleMouseRelease(event->pos());
        //releaseMouseAndDontTrack();
        return;
    }

    grabMouseAndTrack();

    mDoubleClick = false;
    //mMovesToSkip = 2;
    mFirstMouseMove = true;

    mLastPressPosRel = mLastMouseEventPosRel;
    const qreal invScale = 1/mCanvasTransform.m11();
    mLastPressedPoint = getPointAtAbsPos(mLastMouseEventPosRel,
                                         mCurrentMode, invScale);

    if(mRotPivot->handleMousePress(mLastMouseEventPosRel,
                                   mCurrentMode, invScale)) return;
    if(mCurrentMode == CanvasMode::MOVE_BOX) {
        if(mHoveredPoint_d) {
            handleMovePointMousePressEvent();
        } else {
            handleMovePathMousePressEvent();
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddSmartPointMousePress();
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMousePressEvent();
    } else if(mCurrentMode == CanvasMode::PICK_PAINT_SETTINGS) {
        mLastPressedBox = getBoxAtFromAllDescendents(mLastPressPosRel);
    } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
        const auto newPath = SPtrCreate(Circle)();
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(mLastMouseEventPosRel);
        //newPath->startAllPointsTransform();
        clearBoxesSelection();
        addBoxToSelection(newPath.get());

        mCurrentCircle = newPath.get();

    } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
        const auto newPath = SPtrCreate(Rectangle)();
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(mLastMouseEventPosRel);
        //newPath->startAllPointsTransform();
        clearBoxesSelection();
        addBoxToSelection(newPath.get());

        mCurrentRectangle = newPath.get();
    } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
        const auto newPath = SPtrCreate(TextBox)();
        const FontsWidget * const fonstWidget =
                mMainWindow->getFontsWidget();
        newPath->setSelectedFontFamilyAndStyle(
                    fonstWidget->getCurrentFontFamily(),
                    fonstWidget->getCurrentFontStyle());
        newPath->setSelectedFontSize(fonstWidget->getCurrentFontSize());
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(mLastMouseEventPosRel);

        mCurrentTextBox = newPath.get();

        clearBoxesSelection();
        addBoxToSelection(newPath.get());
    } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
        //setCanvasMode(CanvasMode::MOVE_POINT);
        const auto partBox = SPtrCreate(ParticleBox)();
        mCurrentBoxesGroup->addContainedBox(partBox);
        partBox->setAbsolutePos(mLastMouseEventPosRel);
        clearBoxesSelection();
        addBoxToSelection(partBox.get());

        mLastPressedPoint = partBox->getBottomRightPoint();
    } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_EMITTER) {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isParticleBox()) {
                if(box->absPointInsidePath(mLastMouseEventPosRel)) {
                    const auto particleBox = GetAsPtr(box, ParticleBox);
                    particleBox->addEmitterAtAbsPos(mLastMouseEventPosRel);
                    break;
                }
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_PAINT_BOX) {
        //setCanvasMode(CanvasMode::MOVE_POINT);
        const auto paintBox = SPtrCreate(PaintBox)();
        mCurrentBoxesGroup->addContainedBox(paintBox);
        paintBox->setAbsolutePos(mLastMouseEventPosRel);
        clearBoxesSelection();
        clearPointsSelection();
        addBoxToSelection(paintBox.get());
    }
}

QPointF Canvas::mapCanvasAbsToRel(const QPointF &pos) {
    return mCanvasTransform.inverted().map(pos);
}

void Canvas::setLastMouseEventPosAbs(const QPointF &abs) {
    mLastMouseEventPosAbs = abs;
    mLastMouseEventPosRel = mapCanvasAbsToRel(mLastMouseEventPosAbs);
}

void Canvas::setLastMousePressPosAbs(const QPointF &abs) {
    mLastPressPosAbs = abs;
    mLastPressPosRel = mapCanvasAbsToRel(mLastMouseEventPosAbs);
}

void Canvas::setCurrentMouseEventPosAbs(const QPointF &abs) {
    mCurrentMouseEventPosAbs = abs;
    mCurrentMouseEventPosRel = mapCanvasAbsToRel(mCurrentMouseEventPosAbs);
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mCurrentNormalSegment.isValid()) {
            mCurrentNormalSegment.cancelPassThroughTransform();
        } else {
            cancelSelectedPointsTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
    } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
        if(mRotPivot->isSelected()) {
            mRotPivot->cancelTransform();
        } else {
            cancelSelectedBoxesTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {

    } else if(mCurrentMode == CanvasMode::ADD_POINT) {

    } else if(mCurrentMode == PICK_PAINT_SETTINGS) {
        //mCanvasWindow->setCanvasMode(MOVE_PATH);
    }

    if(mIsMouseGrabbing) releaseMouseAndDontTrack();
}

void Canvas::handleMovePointMouseRelease() {
    if(mRotPivot->isSelected()) {
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        finishSelectedPointsTransform();
    } else if(mSelecting) {
        mSelecting = false;
        if(!isShiftPressed()) clearPointsSelection();
        moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        selectAndAddContainedPointsToSelection(mSelectionRect);
    } else if(mFirstMouseMove) {
        if(mLastPressedPoint) {
            if(isShiftPressed()) {
                if(mLastPressedPoint->isSelected()) {
                    removePointFromSelection(mLastPressedPoint);
                } else {
                    addPointToSelection(mLastPressedPoint);
                }
            } else {
                selectOnlyLastPressedPoint();
            }
        } else {
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(
                        mCurrentMouseEventPosRel);
            if(!mLastPressedBox ? true : mLastPressedBox->SWT_isContainerBox()) {
                BoundingBox * const pressedBox =
                        getBoxAtFromAllDescendents(mCurrentMouseEventPosRel);
                if(!pressedBox) {
                    if(!isShiftPressed()) {
                        clearPointsSelectionOrDeselect();
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentSmartEndPoint();
                    clearLastPressedPoint();
                    setCurrentBoxesGroup(pressedBox->getParentGroup());
                    addBoxToSelection(pressedBox);
                    mLastPressedBox = pressedBox;
                }
            }
            if(mLastPressedBox) {
                if(isShiftPressed()) {
                    if(mLastPressedBox->isSelected()) {
                        removeBoxFromSelection(mLastPressedBox);
                    } else {
                        addBoxToSelection(mLastPressedBox);
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentSmartEndPoint();
                    clearLastPressedPoint();
                    selectOnlyLastPressedBox();
                }
            }
        }
    } else {
        finishSelectedPointsTransform();
        if(mLastPressedPoint) {
            if(!mLastPressedPoint->selectionEnabled()) {
                removePointFromSelection(mLastPressedPoint);
            }
        }
    }
}

void Canvas::handleMovePathMouseRelease() {
    if(mRotPivot->isSelected()) {
        if(!mFirstMouseMove) mRotPivot->finishTransform();
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling()) {
        mRotPivot->handleMouseRelease();
        finishSelectedBoxesTransform();
    } else if(mFirstMouseMove) {
        mSelecting = false;
        if(isShiftPressed() && mLastPressedBox) {
            if(mLastPressedBox->isSelected()) {
                removeBoxFromSelection(mLastPressedBox);
            } else {
                addBoxToSelection(mLastPressedBox);
            }
        } else {
            selectOnlyLastPressedBox();
        }
    } else if(mSelecting) {
        moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        finishSelectedBoxesTransform();
    }
}

void Canvas::handleMouseRelease() {
    if(mIsMouseGrabbing) releaseMouseAndDontTrack();
    if(mCurrentNormalSegment.isValid()) {
        if(!mFirstMouseMove) mCurrentNormalSegment.finishPassThroughTransform();
        mHoveredNormalSegment = mCurrentNormalSegment;
        mHoveredNormalSegment.generateSkPath();
        mCurrentNormalSegment.reset();
        return;
    }
    if(!mDoubleClick) {
        if(mCurrentMode == CanvasMode::MOVE_POINT ||
           mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
            handleMovePointMouseRelease();
            if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
                mCanvasWindow->setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
            }
        } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
            if(!mLastPressedPoint) {
                handleMovePathMouseRelease();
            } else {
                handleMovePointMouseRelease();
                clearPointsSelection();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddSmartPointMouseRelease();
        } else if(mCurrentMode == PICK_PAINT_SETTINGS) {
            if(mLastPressedBox) {
                const auto srcPathBox = GetAsPtr(mLastPressedBox, PathBox);
                for(const auto& box : mSelectedBoxes) {
                    if(box->SWT_isPathBox()) {
                        const auto pathBox = GetAsPtr(box, PathBox);
                        if(isCtrlPressed()) {
                            if(isShiftPressed()) {
                                pathBox->duplicateStrokeSettingsFrom(
                                            srcPathBox->getStrokeSettings());
                                pathBox->resetStrokeGradientPointsPos();
                            } else {
                                pathBox->duplicateFillSettingsFrom(
                                            srcPathBox->getFillSettings());
                                pathBox->resetFillGradientPointsPos();
                            }
                        } else {
                            if(isShiftPressed()) {
                                pathBox->duplicateStrokeSettingsNotAnimatedFrom(
                                            srcPathBox->getStrokeSettings());
                                pathBox->resetStrokeGradientPointsPos();
                            } else {
                                pathBox->duplicateFillSettingsNotAnimatedFrom(
                                            srcPathBox->getFillSettings());
                                pathBox->resetFillGradientPointsPos();
                            }
                        }
                    }
                }
            }
            //mCanvasWindow->setCanvasMode(MOVE_PATH);
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
            if(mCurrentTextBox) {
                mCurrentTextBox->openTextEditor(mMainWindow);
            }
        } else if(mCurrentMode == CanvasMode::ADD_PAINT_BOX) {
            mCanvasWindow->setCanvasMode(CanvasMode::PAINT_MODE);
        }
    }
}

QPointF Canvas::getMoveByValueForEventPos(const QPointF &eventPos) {
    QPointF moveByPoint = eventPos - mLastPressPosRel;
    if(mValueInput.inputEnabled()) {
        moveByPoint = QPointF(mValueInput.getValue(),
                              mValueInput.getValue());
    }
    if(mYOnlyTransform) moveByPoint.setX(0);
    else if(mXOnlyTransform) moveByPoint.setY(0);
    return moveByPoint;
}
#include <QApplication>
#include "MovablePoints/smartctrlpoint.h"
#include "MovablePoints/pathpointshandler.h"
void Canvas::handleMovePointMouseMove() {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) mRotPivot->startTransform();
        mRotPivot->moveByAbs(getMoveByValueForEventPos(mCurrentMouseEventPosRel));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
           mRotPivot->handleMouseMove(mCurrentMouseEventPosRel,
                                      mLastPressPosRel,
                                      mXOnlyTransform, mYOnlyTransform,
                                      mValueInput.inputEnabled(),
                                      mValueInput.getValue(),
                                      mFirstMouseMove,
                                      mCurrentMode);
    } else if(mCurrentNormalSegment.isValid()) {
        if(mFirstMouseMove) mCurrentNormalSegment.startPassThroughTransform();
        mCurrentNormalSegment.makePassThroughAbs(mCurrentMouseEventPosRel,
                                                 mCurrentNormalSegmentT);
    } else {
        if(mLastPressedPoint) {
            addPointToSelection(mLastPressedPoint);

            const auto keyMods = QApplication::queryKeyboardModifiers();
            const bool ctrlPressed = keyMods.testFlag(Qt::ControlModifier);
            if(ctrlPressed && mLastPressedPoint->isSmartNodePoint()) {
                const auto nodePt = GetAsPtr(mLastPressedPoint,
                                             SmartNodePoint);
                if(nodePt->isDissolved()) {
                    const int selId = nodePt->moveToClosestSegment(
                                mCurrentMouseEventPosRel);
                    const auto handler = nodePt->getHandler();
                    const auto dissPt = handler->getPointWithId<SmartNodePoint>(selId);
                    if(nodePt->getNodeId() != selId) {
                        removePointFromSelection(nodePt);
                        addPointToSelection(dissPt);
                    }
                    mLastPressedPoint = dissPt;
                    return;
                }
            }

            if(!mLastPressedPoint->selectionEnabled()) {
                if(mFirstMouseMove) mLastPressedPoint->startTransform();
                mLastPressedPoint->moveByAbs(
                        getMoveByValueForEventPos(mCurrentMouseEventPosRel));
                return;
            }
        }
        moveSelectedPointsByAbs(
                    getMoveByValueForEventPos(mCurrentMouseEventPosRel),
                    mFirstMouseMove);
    }
}

void Canvas::handleMovePathMouseMove() {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) mRotPivot->startTransform();

        mRotPivot->moveByAbs(getMoveByValueForEventPos(mCurrentMouseEventPosRel));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseMove(mCurrentMouseEventPosRel,
                                   mLastPressPosRel,
                                   mXOnlyTransform, mYOnlyTransform,
                                   mValueInput.inputEnabled(),
                                   mValueInput.getValue(),
                                   mFirstMouseMove,
                                   mCurrentMode);
    } else {
        if(mLastPressedBox) {
            addBoxToSelection(mLastPressedBox);
            mLastPressedBox = nullptr;
        }

        const auto moveBy = getMoveByValueForEventPos(mCurrentMouseEventPosRel);
        moveSelectedBoxesByAbs(moveBy, mFirstMouseMove);
    }
}

void Canvas::updateTransformation() {
    if(mSelecting) {
        moveSecondSelectionPoint(mLastMouseEventPosRel);
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseMove();
    } else if(isMovingPath()) {
        if(!mLastPressedPoint) {
            handleMovePathMouseMove();
        } else {
            handleMovePointMouseMove();
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddSmartPointMouseMove();
    }
}
