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
#include "GUI/canvaswindow.h"
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

void Canvas::handleMovePathMousePressEvent(const MouseEvent& e) {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(e.fPos);
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

    const auto &listOfCanvas = mActiveWindow->getCanvasList();
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
        addPathEffect(SPtrCreate(DisplacePathEffect)());
    });

    pathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addPathEffect(SPtrCreate(DuplicatePathEffect)());
    });

    pathEffectsMenu->addAction("Sub-Path Effect", [this]() {
        addPathEffect(SPtrCreate(SubPathEffect)());
    });

    pathEffectsMenu->addAction("Solidify Effect", [this]() {
        addPathEffect(SPtrCreate(SolidifyPathEffect)());
    });

    pathEffectsMenu->addAction("Sum Effect", [this]() {
        addPathEffect(SPtrCreate(SumPathEffect)());
    });
//    pathEffectsMenu->addAction("Operation Effect");
//    pathEffectsMenu->addAction("Group Sum Effect");

    QMenu * const fillPathEffectsMenu = menu->addMenu("Fill Effects");

    fillPathEffectsMenu->addAction("Displace Effect", [this]() {
        addFillPathEffect(SPtrCreate(DisplacePathEffect)());
    });

    fillPathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addFillPathEffect(SPtrCreate(DuplicatePathEffect)());
    });

    fillPathEffectsMenu->addAction("Solidify Effect", [this]() {
        addFillPathEffect(SPtrCreate(SolidifyPathEffect)());
    });

    fillPathEffectsMenu->addAction("Sum Effect", [this]() {
        addFillPathEffect(SPtrCreate(SumPathEffect)());
    });
//    fillPathEffectsMenu->addAction("Operation Effect");

    QMenu * const outlinePathEffectsMenu = menu->addMenu("Outline Effects");

    outlinePathEffectsMenu->addAction("Displace Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DisplacePathEffect)());
    });

    outlinePathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DuplicatePathEffect)());
    });

    pathEffectsMenu->addAction("Sub-Path Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SubPathEffect)());
    });

    outlinePathEffectsMenu->addAction("Solidify Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SolidifyPathEffect)());
    });

    outlinePathEffectsMenu->addAction("Sum Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SumPathEffect)());
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
        mActiveWindow->openSettingsWindowForCurrentCanvas();
    });
}

void Canvas::handleRightButtonMousePress(const QPoint& globalPos) {
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
            qMenu.exec(globalPos);
        } else if(mLastPressedBox) {
            if(!mLastPressedBox->isSelected()) {
                if(!isShiftPressed()) clearBoxesSelection();
                addBoxToSelection(mLastPressedBox);
            }

            QMenu qMenu;
            addSelectedBoxesActions(&qMenu);
            qMenu.exec(globalPos);
        } else {
            clearPointsSelection();
            clearBoxesSelection();
            QMenu menu(mActiveWindow->getCanvasWidget());
            addActionsToMenu(&menu);
            menu.exec(globalPos);
        }
    }
}

void Canvas::clearHoveredEdge() {
    mHoveredNormalSegment.reset();
}

void Canvas::handleMovePointMousePressEvent(const MouseEvent& e) {
    if(mHoveredNormalSegment.isValid()) {
        if(isCtrlPressed()) {
            clearPointsSelection();
            mLastPressedPoint = mHoveredNormalSegment.divideAtAbsPos(e.fPos);
        } else {
            mCurrentNormalSegment = mHoveredNormalSegment;
            mCurrentNormalSegmentT = mCurrentNormalSegment.closestAbsT(e.fPos);
            clearPointsSelection();
            clearCurrentSmartEndPoint();
            clearLastPressedPoint();
        }
        clearHovered();
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


void Canvas::handleLeftButtonMousePress(const MouseEvent& e) {
    if(e.fMouseGrabbing) {
        //handleMouseRelease(event->pos());
        //releaseMouseAndDontTrack();
        return;
    }

    mDoubleClick = false;
    //mMovesToSkip = 2;
    mFirstMouseMove = true;

    const qreal invScale = 1/e.fScale;
    mLastPressedPoint = getPointAtAbsPos(e.fPos, e.fMode, invScale);

    if(mRotPivot->isPointAtAbsPos(e.fPos, e.fMode, invScale)) {
        return mRotPivot->select();
    }
    if(e.fMode == CanvasMode::MOVE_BOX) {
        if(mHoveredPoint_d) {
            handleMovePointMousePressEvent(e);
        } else {
            handleMovePathMousePressEvent(e);
        }
    } else if(e.fMode == CanvasMode::ADD_POINT) {
        handleAddSmartPointMousePress(e);
    } else if(e.fMode == CanvasMode::MOVE_POINT) {
        handleMovePointMousePressEvent(e);
    } else if(e.fMode == CanvasMode::PICK_PAINT_SETTINGS) {
        mLastPressedBox = getBoxAtFromAllDescendents(e.fPos);
    } else if(e.fMode == CanvasMode::ADD_CIRCLE) {
        const auto newPath = SPtrCreate(Circle)();
        newPath->planCenterPivotPosition();
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(e.fPos);
        //newPath->startAllPointsTransform();
        clearBoxesSelection();
        addBoxToSelection(newPath.get());

        mCurrentCircle = newPath.get();

    } else if(e.fMode == CanvasMode::ADD_RECTANGLE) {
        const auto newPath = SPtrCreate(Rectangle)();
        newPath->planCenterPivotPosition();
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(e.fPos);
        //newPath->startAllPointsTransform();
        clearBoxesSelection();
        addBoxToSelection(newPath.get());

        mCurrentRectangle = newPath.get();
    } else if(e.fMode == CanvasMode::ADD_TEXT) {
        const auto newPath = SPtrCreate(TextBox)();
        newPath->planCenterPivotPosition();
        const FontsWidget * const fonstWidget =
                mMainWindow->getFontsWidget();
        newPath->setSelectedFontFamilyAndStyle(
                    fonstWidget->getCurrentFontFamily(),
                    fonstWidget->getCurrentFontStyle());
        newPath->setSelectedFontSize(fonstWidget->getCurrentFontSize());
        mCurrentBoxesGroup->addContainedBox(newPath);
        newPath->setAbsolutePos(e.fPos);

        mCurrentTextBox = newPath.get();

        clearBoxesSelection();
        addBoxToSelection(newPath.get());
    } else if(e.fMode == CanvasMode::ADD_PARTICLE_BOX) {
        //setCanvasMode(CanvasMode::MOVE_POINT);
        const auto partBox = SPtrCreate(ParticleBox)();
        mCurrentBoxesGroup->addContainedBox(partBox);
        partBox->setAbsolutePos(e.fPos);
        clearBoxesSelection();
        addBoxToSelection(partBox.get());

        mLastPressedPoint = partBox->getBottomRightPoint();
    } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_EMITTER) {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isParticleBox()) {
                if(box->absPointInsidePath(e.fPos)) {
                    const auto particleBox = GetAsPtr(box, ParticleBox);
                    particleBox->addEmitterAtAbsPos(e.fPos);
                    break;
                }
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_PAINT_BOX) {
        //setCanvasMode(CanvasMode::MOVE_POINT);
        const auto paintBox = SPtrCreate(PaintBox)();
        paintBox->planCenterPivotPosition();
        mCurrentBoxesGroup->addContainedBox(paintBox);
        paintBox->setAbsolutePos(e.fPos);
        clearBoxesSelection();
        clearPointsSelection();
        addBoxToSelection(paintBox.get());
    }
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mCurrentNormalSegment.isValid()) {
            mCurrentNormalSegment.cancelPassThroughTransform();
        } else {
            cancelSelectedPointsTransform();
        }
    } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
        if(mRotPivot->isSelected()) {
            mRotPivot->cancelTransform();
        } else {
            cancelSelectedBoxesTransform();
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {

    } else if(mCurrentMode == PICK_PAINT_SETTINGS) {
        //mCanvasWindow->setCanvasMode(MOVE_PATH);
    }

    if(mIsMouseGrabbing) releaseMouseAndDontTrack();
}

void Canvas::handleMovePointMouseRelease(const MouseEvent &e) {
    if(mRotPivot->isSelected()) {
        mRotPivot->deselect();
    } else if(mTransMode == MODE_ROTATE || mTransMode == MODE_SCALE) {
        finishSelectedPointsTransform();
        mTransMode = MODE_NONE;
    } else if(mSelecting) {
        mSelecting = false;
        if(!isShiftPressed()) clearPointsSelection();
        moveSecondSelectionPoint(e.fPos);
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
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(e.fPos);
            if(!mLastPressedBox ? true : mLastPressedBox->SWT_isContainerBox()) {
                const auto pressedBox = getBoxAtFromAllDescendents(e.fPos);
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

void Canvas::handleMovePathMouseRelease(const MouseEvent &e) {
    if(mRotPivot->isSelected()) {
        if(!mFirstMouseMove) mRotPivot->finishTransform();
        mRotPivot->deselect();
    } else if(mTransMode == MODE_ROTATE || mTransMode == MODE_SCALE) {
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
        moveSecondSelectionPoint(e.fPos);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        finishSelectedBoxesTransform();
    }
}

void Canvas::handleMouseRelease(const MouseEvent &e) {
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
            handleMovePointMouseRelease(e);
            if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
                mActiveWindow->setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
            }
        } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
            if(!mLastPressedPoint) {
                handleMovePathMouseRelease(e);
            } else {
                handleMovePointMouseRelease(e);
                clearPointsSelection();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddSmartPointMouseRelease(e);
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
            mActiveWindow->setCanvasMode(CanvasMode::PAINT_MODE);
        }
    }
}

QPointF Canvas::getMoveByValueForEvent(const MouseEvent &e) {
    if(mValueInput.inputEnabled())
        return mValueInput.getPtValue();
    const QPointF moveByPoint = e.fPos - e.fLastPressPos;
    mValueInput.setDisplayedValue(moveByPoint);
    if(mValueInput.yOnlyMode()) return {0, moveByPoint.y()};
    else if(mValueInput.xOnlyMode()) return {moveByPoint.x(), 0};
    return moveByPoint;
}

#include <QApplication>
#include "MovablePoints/smartctrlpoint.h"
#include "MovablePoints/pathpointshandler.h"
void Canvas::handleMovePointMouseMove(const MouseEvent &e) {
    if(mTransMode == MODE_SCALE) {
        scaleSelected(e);
    } else if(mTransMode == MODE_ROTATE) {
        rotateSelected(e);
    } else if(mCurrentNormalSegment.isValid()) {
        if(mFirstMouseMove) mCurrentNormalSegment.startPassThroughTransform();
        mCurrentNormalSegment.makePassThroughAbs(e.fPos, mCurrentNormalSegmentT);
    } else {
        if(mLastPressedPoint) {
            addPointToSelection(mLastPressedPoint);

            const auto keyMods = QApplication::queryKeyboardModifiers();
            const bool ctrlPressed = keyMods.testFlag(Qt::ControlModifier);
            if(ctrlPressed && mLastPressedPoint->isSmartNodePoint()) {
                const auto nodePt = GetAsPtr(mLastPressedPoint,
                                             SmartNodePoint);
                if(nodePt->isDissolved()) {
                    const int selId = nodePt->moveToClosestSegment(e.fPos);
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
                mLastPressedPoint->moveByAbs(getMoveByValueForEvent(e));
                return;
            }
        }
        moveSelectedPointsByAbs(getMoveByValueForEvent(e),
                                mFirstMouseMove);
    }
}

void Canvas::scaleSelected(const MouseEvent& e) {
    const QPointF absPos = mRotPivot->getAbsolutePos();
    const QPointF distMoved = e.fPos - e.fLastPressPos;

    qreal scaleBy;
    if(mValueInput.inputEnabled()) {
        scaleBy = mValueInput.getValue();
    } else {
        scaleBy = 1 + distSign(distMoved)*0.003;
    }
    qreal scaleX;
    qreal scaleY;
    if(mValueInput.xOnlyMode()) {
        scaleX = scaleBy;
        scaleY = 1;
    } else if(mValueInput.yOnlyMode()) {
        scaleX = 1;
        scaleY = scaleBy;
    } else {
        scaleX = scaleBy;
        scaleY = scaleBy;
    }

    if(e.fMode == CanvasMode::MOVE_BOX) {
        scaleSelectedBy(scaleX, scaleY, absPos, mFirstMouseMove);
    } else {
        scaleSelectedPointsBy(scaleX, scaleY, absPos, mFirstMouseMove);
    }

    if(!mValueInput.inputEnabled())
        mValueInput.setDisplayedValue({scaleX, scaleY});
    mRotPivot->setMousePos(e.fPos);
}

void Canvas::rotateSelected(const MouseEvent& e) {
    const QPointF absPos = mRotPivot->getAbsolutePos();
    qreal rot;
    if(mValueInput.inputEnabled()) {
        rot = mValueInput.getValue();
    } else {
        const QLineF dest_line(absPos, e.fPos);
        const QLineF prev_line(absPos, e.fLastPressPos);
        qreal d_rot = dest_line.angleTo(prev_line);
        if(d_rot > 180) d_rot -= 360;

        if(mLastDRot - d_rot > 90) {
            mRotHalfCycles += 2;
        } else if(mLastDRot - d_rot < -90) {
            mRotHalfCycles -= 2;
        }
        mLastDRot = d_rot;
        rot = d_rot + mRotHalfCycles*180;
    }

    if(mCurrentMode == CanvasMode::MOVE_BOX) {
        rotateSelectedBy(rot, absPos, mFirstMouseMove);
    } else {
        rotateSelectedPointsBy(rot, absPos, mFirstMouseMove);
    }

    if(!mValueInput.inputEnabled())
        mValueInput.setDisplayedValue(rot);
    mRotPivot->setMousePos(e.fPos);
}

void Canvas::handleMovePathMouseMove(const MouseEvent& e) {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) mRotPivot->startTransform();
        mRotPivot->moveByAbs(getMoveByValueForEvent(e));
    } else if(mTransMode == MODE_SCALE) {
        scaleSelected(e);
    } else if(mTransMode == MODE_ROTATE) {
        rotateSelected(e);
    } else {
        if(mLastPressedBox) {
            addBoxToSelection(mLastPressedBox);
            mLastPressedBox = nullptr;
        }

        const auto moveBy = getMoveByValueForEvent(e);
        moveSelectedBoxesByAbs(moveBy, mFirstMouseMove);
    }
}

void Canvas::updateTransformation(const KeyEvent &e) {
    if(mSelecting) {
        moveSecondSelectionPoint(e.fPos);
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseMove(e);
    } else if(mCurrentMode == CanvasMode::MOVE_BOX) {
        if(!mLastPressedPoint) {
            handleMovePathMouseMove(e);
        } else {
            handleMovePointMouseMove(e);
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddSmartPointMouseMove(e);
    }
}
