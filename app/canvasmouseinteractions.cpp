#include "canvas.h"
#include <QMouseEvent>
#include <QMenu>
#include "MovablePoints/pathpivot.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/imagebox.h"
#include "Boxes/textbox.h"
#include "Boxes/linkbox.h"
#include "edge.h"
#include "MovablePoints/nodepoint.h"
#include "Animators/pathanimator.h"
#include "pointhelpers.h"
#include "Boxes/particlebox.h"
#include "clipboardcontainer.h"
#include "GUI/mainwindow.h"
#include "Boxes/paintbox.h"
#include "Paint/brush.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "GUI/fontswidget.h"
#include "Boxes/bone.h"
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
#include <QFileDialog>
#include "GUI/paintboxsettingsdialog.h"
#include "GUI/customfpsdialog.h"
#include "Boxes/vectorpath.h"
#include "GPUEffects/gpurastereffect.h"

void Canvas::handleMovePathMousePressEvent() {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPosRel);
    mLastPressedBone = nullptr;
    if(!mLastPressedBox) {
        if(!isShiftPressed()) clearBoxesSelection();
    } else {
        if(mLastPressedBox->SWT_isBonesBox()) {
            if(mBonesSelectionEnabled) {
                mLastPressedBone = GetAsPtr(mLastPressedBox, BonesBox)->
                        getBoneAtAbsPos(mLastMouseEventPosRel);
                if(!isShiftPressed()) clearBonesSelection();
                return;
            }
        }
        if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
            clearBoxesSelection();
        }
    }
}

void Canvas::addSelectedBoxesActions(QMenu * const menu) {
    bool hasVectorPathBox = false;
    for(const auto& box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            hasVectorPathBox = true;
            break;
        }
    }
    bool hasGroups = false;
    for(const auto& box : mSelectedBoxes) {
        if(box->SWT_isBoxesGroup()) {
            hasGroups = true;
            break;
        }
    }
    bool hasPathBox = false;
    if(hasVectorPathBox) {
        hasPathBox = true;
    } else {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isPathBox()) {
                hasPathBox = true;
                break;
            }
        }
    }
    if(hasVectorPathBox || hasGroups) {
        menu->addAction("Apply Transformation", [this]() {
            applyCurrentTransformation();
        });
    }
    menu->addSeparator();
    menu->addAction("Create Link", [this]() {
        mCurrentBoxesGroup->addContainedBox(createLink());
    });
    menu->addAction("Center Pivot", [this]() {
        centerPivotForSelected();
    });
    menu->addSeparator();

    QAction * const copyAction = menu->addAction("Copy", [this]() {
        this->copyAction();
    });
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);

    QAction * const cutAction = menu->addAction("Cut", [this]() {
        this->cutAction();
    });
    cutAction->setShortcut(Qt::CTRL + Qt::Key_X);

    QAction * const duplicateAction = menu->addAction("Duplicate", [this]() {
        this->duplicateSelectedBoxes();
    });
    duplicateAction->setShortcut(Qt::CTRL + Qt::Key_D);

    QAction * const deleteAction = menu->addAction("Delete", [this]() {
        this->removeSelectedBoxesAndClearList();
    });
    deleteAction->setShortcut(Qt::Key_Delete);

    menu->addSeparator();

    QAction * const groupAction = menu->addAction("Group", [this]() {
        this->groupSelectedBoxes();
    });
    groupAction->setShortcut(Qt::CTRL + Qt::Key_G);


    QAction * const ungroupAction = menu->addAction("Ungroup", [this]() {
        this->ungroupSelectedBoxes();
    });
    ungroupAction->setEnabled(hasGroups);
    ungroupAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    menu->addSeparator();

    QMenu * const effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur", [this]() {
        this->applyEffectToSelected<BlurEffect>();
    });
    effectsMenu->addAction("Motion Blur", [this]() {
        this->applySampledMotionBlurToSelected();
    });
    effectsMenu->addAction("Shadow", [this]() {
        this->applyEffectToSelected<ShadowEffect>();
    });
//    effectsMenu->addAction("Lines");
//    effectsMenu->addAction("Circles");
//    effectsMenu->addAction("Swirl");
//    effectsMenu->addAction("Oil");
//    effectsMenu->addAction("Implode");
    effectsMenu->addAction("Desaturate", [this]() {
        this->applyEffectToSelected<DesaturateEffect>();
    });

    effectsMenu->addAction("Colorize", [this]() {
        this->applyEffectToSelected<ColorizeEffect>();
    });

    effectsMenu->addAction("Contrast", [this]() {
        this->applyEffectToSelected<ContrastEffect>();
    });

    effectsMenu->addAction("Brightness", [this]() {
        this->applyEffectToSelected<BrightnessEffect>();
    });

    effectsMenu->addAction("Replace Color", [this]() {
        this->applyEffectToSelected<ReplaceColorEffect>();
    });

    QMenu * const gpuEffectsMenu = menu->addMenu("GPU Effects");
    for(const auto& effect : GPURasterEffectCreator::sEffectCreators) {
        gpuEffectsMenu->addAction(effect->fName, [this, effect]() {
            applyGPURasterEffectToSelected(effect);
        });
    }

    if(hasPathBox || hasGroups) {
        QMenu * const pathEffectsMenu = menu->addMenu("Path Effects");

        pathEffectsMenu->addAction("Displace Effect", [this]() {
            this->applyDiscretePathEffectToSelected();
        });

        pathEffectsMenu->addAction("Duplicate Effect", [this]() {
            this->applyDuplicatePathEffectToSelected();
        });

        pathEffectsMenu->addAction("Length Effect", [this]() {
            this->applyLengthPathEffectToSelected();
        });

        pathEffectsMenu->addAction("Solidify Effect", [this]() {
            this->applySolidifyPathEffectToSelected();
        });

        pathEffectsMenu->addAction("Operation Effect", [this]() {
            this->applySumPathEffectToSelected();
        });

        if(hasGroups) {
            pathEffectsMenu->addAction("Group Sum Effect", [this]() {
                this->applyGroupSumPathEffectToSelected();
            });
        }

        QMenu * const fillPathEffectsMenu = menu->addMenu("Fill Effects");

        fillPathEffectsMenu->addAction("Displace Effect", [this]() {
            this->applyDiscreteFillPathEffectToSelected();
        });

        fillPathEffectsMenu->addAction("Duplicate Effect", [this]() {
            this->applyDuplicateFillPathEffectToSelected();
        });

//        fillPathEffectsMenu->addAction("Solidify Effect", [this]() {
//            this->applySolidifyFillPathEffectToSelected();
//        });

        fillPathEffectsMenu->addAction("Operation Effect", [this]() {
            this->applySumFillPathEffectToSelected();
        });

        QMenu * const outlinePathEffectsMenu = menu->addMenu("Outline Effects");
        outlinePathEffectsMenu->addAction("Displace Effect", [this]() {
            this->applyDiscreteOutlinePathEffectToSelected();
        });

        outlinePathEffectsMenu->addAction("Duplicate Effect", [this]() {
            this->applyDuplicateOutlinePathEffectToSelected();
        });

//        outlinePathEffectsMenu->addAction("Length Effect", [this]() {
//            this->applyLengthOutlinePathEffectToSelected();
//        });

//        outlinePathEffectsMenu->addAction("Solidify Effect", [this]() {
//            this->applySolidifyOutlinePathEffectToSelected();
//        });

//        outlinePathEffectsMenu->addAction("Operation Effect", [this]() {
//            this->applySumOutlinePathEffectToSelected();
//        });
    }

    for(const auto& box : mSelectedBoxes) {
        if(box->SWT_isPaintBox()) {
            const auto paintBox = GetAsPtr(box, PaintBox);
            menu->addSeparator();
            menu->addAction("New Paint Frame", [this]() {
                for(const auto& box : mSelectedBoxes) {
                    if(box->SWT_isPaintBox()) {
                        const auto paintBox = GetAsPtr(box, PaintBox);
                        paintBox->newPaintFrameOnCurrentFrame();
                    }
                }
            });
            menu->addAction("New Empty Paint Frame", [this]() {
                for(const auto& box : mSelectedBoxes) {
                    if(box->SWT_isPaintBox()) {
                        PaintBox* paintBox = GetAsPtr(box, PaintBox);
                        paintBox->newEmptyPaintFrameOnCurrentFrame();
                    }
                }
            });
            menu->addAction("Setup Animation Frames", [this]() {
                PaintBoxSettingsDialog dialog;
                const auto firstPaintBox =
                        GetAsPtr(mSelectedBoxes.first(), PaintBox);
                int frameStep = firstPaintBox->getFrameStep();
                int overlapFrames = firstPaintBox->getOverlapFrames();
                dialog.setOverlapFrames(overlapFrames);
                dialog.setFrameStep(frameStep);
                dialog.exec();
                if(dialog.result() == QDialog::Rejected) return;
                frameStep = dialog.getFrameStep();
                overlapFrames = dialog.getOverlapFrames();
                for(const auto& box : mSelectedBoxes) {
                    if(box->SWT_isPaintBox()) {
                        const auto paintBox = GetAsPtr(box, PaintBox);
                        paintBox->setOverlapFrames(overlapFrames);
                        paintBox->setFrameStep(frameStep);
                    }
                }
            });
            menu->addSeparator();
            if(paintBox->isAnimated()) {
                QAction * const draftAction = menu->addAction("Draft", [this]() {
                    for(const auto& box : mSelectedBoxes) {
                        if(box->SWT_isPaintBox()) {
                            const auto paintBox = GetAsPtr(box, PaintBox);
                            paintBox->setIsDraft(!paintBox->isDraft());
                        }
                    }
                });
                draftAction->setCheckable(true);
                draftAction->setChecked(paintBox->isDraft());
            }
            menu->addSeparator();
            menu->addAction("Load From Image", [this]() {
                MainWindow::getInstance()->disableEventFilter();
                const QString importPath = QFileDialog::getOpenFileName(
                                                MainWindow::getInstance(),
                                                "Load From Image", "",
                                                "Image Files (*.png *.jpg)");
                MainWindow::getInstance()->enableEventFilter();
                if(!importPath.isEmpty()) {
                    QImage img;
                    if(img.load(importPath)) {
                        for(const auto& box : mSelectedBoxes) {
                            if(box->SWT_isPaintBox()) {
                                const auto paintBox = GetAsPtr(box, PaintBox);
                                paintBox->loadFromImage(img);
                            }
                        }
                    }
                }
            });

            break;
        }
    }
}

#include <QInputDialog>
#include "PathEffects/patheffect.h"
void Canvas::addActionsToMenu(QMenu * const menu,
                              QWidget* const widgetsParent) {
    Q_UNUSED(widgetsParent);
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

    pathEffectsMenu->addAction("Length Effect", [this]() {
        addPathEffect(SPtrCreate(LengthPathEffect)(false));
    });

    pathEffectsMenu->addAction("Solidify Effect", [this]() {
        addPathEffect(SPtrCreate(SolidifyPathEffect)(false));
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
//    fillPathEffectsMenu->addAction("Operation Effect");

    QMenu * const outlinePathEffectsMenu = menu->addMenu("Outline Effects");

    outlinePathEffectsMenu->addAction("Displace Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DisplacePathEffect)(true));
    });

    outlinePathEffectsMenu->addAction("Duplicate Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(DuplicatePathEffect)(true));
    });

    pathEffectsMenu->addAction("Length Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(LengthPathEffect)(true));
    });

    outlinePathEffectsMenu->addAction("Solidify Effect", [this]() {
        addOutlinePathEffect(SPtrCreate(SolidifyPathEffect)(true));
    });

//    outlinePathEffectsMenu->addAction("Operation Effect");

    menu->addAction("Map to Different Fps...", [this]() {
        bool ok;
        const qreal newFps = QInputDialog::getDouble(
                    mMainWindow, "Map to Different Fps",
                    "New Fps:", mFps, 1., 999., 2, &ok);
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
        const QPointF eventPos = mapCanvasAbsToRel(event->pos());
        BoundingBox* const pressedBox = mCurrentBoxesGroup->getBoxAt(eventPos);
        if(!pressedBox) {
            clearBoxesSelection();
            QMenu menu(mCanvasWindow->getCanvasWidget());
            addActionsToMenu(&menu, mMainWindow);
            menu.exec(event->globalPos());
        } else {
            if(!pressedBox->isSelected()) {
                if(!isShiftPressed()) clearBoxesSelection();
                addBoxToSelection(pressedBox);
            }

            QMenu menu(mCanvasWindow->getCanvasWidget());
            pressedBox->addActionsToMenu(&menu, mMainWindow);
            addSelectedBoxesActions(&menu);
            menu.exec(event->globalPos());
        }
    }
}

void Canvas::clearHoveredEdge() {
    mHoveredEdge_d = nullptr;
}

void Canvas::handleMovePointMousePressEvent() {
    if(!mLastPressedPoint) {
        if(isCtrlPressed()) {
            clearPointsSelection();
            mLastPressedPoint = createNewPointOnLineNearSelected(
                                        mLastPressPosRel,
                                        isShiftPressed(),
                                        1/mCanvasTransformMatrix.m11());

        } else {
            mCurrentEdge = getEdgeAt(mLastPressPosRel);
            if(mCurrentEdge) {
                clearPointsSelection();
                clearCurrentEndPoint();
                clearLastPressedPoint();
            }
        }
        clearHoveredEdge();
    } else {
        if(mLastPressedPoint->isSelected()) return;
        if(!isShiftPressed() && !mLastPressedPoint->isCtrlPoint()) {
            clearPointsSelection();
        }
        if(mLastPressedPoint->isCtrlPoint()) {
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
    mMovesToSkip = 2;
    mFirstMouseMove = true;

    mLastPressPosRel = mLastMouseEventPosRel;
    mLastPressedPoint = getPointAtAbsPos(mLastMouseEventPosRel,
                                   mCurrentMode,
                                   1/mCanvasTransformMatrix.m11());

    if(mRotPivot->handleMousePress(mLastMouseEventPosRel,
                                   1/mCanvasTransformMatrix.m11())) {
    } else if(isMovingPath()) {
        if(mHoveredPoint_d) {
            handleMovePointMousePressEvent();
        } else {
            handleMovePathMousePressEvent();
        }
    } else {
        if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMousePress();
        } // point adding mode
        else if(mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMousePressEvent();
        } else if(mCurrentMode == CanvasMode::PICK_PAINT_SETTINGS) {
            mLastPressedBox = getPathAtFromAllAncestors(mLastPressPosRel);
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

            mLastPressedPoint = paintBox->getBottomRightPoint();
            addPointToSelection(mLastPressedPoint);
            mLastPressedPoint->startTransform();
        } else if(mCurrentMode == CanvasMode::ADD_BONE) {
            //setCanvasMode(CanvasMode::MOVE_POINT);
            BonePt *bonePt = nullptr;

            if(mLastPressedPoint) {
                if(mLastPressedPoint->isBonePoint()) {
                    bonePt = GetAsPtr(mLastPressedPoint, BonePt);
                    if(!bonePt->getTipBone()) {
                        mLastMouseEventPosRel = bonePt->getAbsolutePos();
                        bonePt = nullptr;
                    }
                }
            }
            Bone *newBone = nullptr;
            if(!bonePt) {
                qsptr<BonesBox> boneBox;
                if(mSelectedBoxes.count() > 0) {
                    BoundingBox * const lastSelected = mSelectedBoxes.last();
                    if(lastSelected->SWT_isBonesBox()) {
                        boneBox = GetAsSPtr(lastSelected, BonesBox);

                        newBone = Bone::createBone(boneBox.data());

                        newBone->getRootPt()->setAbsolutePos(
                                    mLastMouseEventPosRel);
                        newBone->getTipPt()->setAbsolutePos(
                                    mLastMouseEventPosRel);
                    }
                }
                if(!boneBox) {
                    boneBox = SPtrCreate(BonesBox)();
                    mCurrentBoxesGroup->addContainedBox(boneBox);
                    boneBox->setAbsolutePos(mLastMouseEventPosRel);
                    clearBoxesSelection();
                    addBoxToSelection(boneBox.get());
                    newBone = Bone::createBone(boneBox.data());
                }
            } else {
                Bone * const boneT = bonePt->getTipBone();
                newBone = Bone::createBone(boneT);
            }
            clearPointsSelection();

            mLastPressedPoint = newBone->getTipPt();
            addPointToSelection(mLastPressedPoint);
            mLastPressedPoint->startTransform();
        }
    } // current mode allows interaction with points
}

QPointF Canvas::mapCanvasAbsToRel(const QPointF &pos) {
    return mCanvasTransformMatrix.inverted().map(pos);
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

void Canvas::setCurrentMousePressPosAbs(const QPointF &abs) {
    mCurrentPressPosAbs = abs;
    mCurrentPressPosRel = mapCanvasAbsToRel(mCurrentMouseEventPosAbs);
}

void Canvas::mousePressEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    setLastMouseEventPosAbs(event->pos());
    if(mCurrentMode == PAINT_MODE) {
        if(mStylusDrawing) return;
        if(event->button() == Qt::LeftButton) {
            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mousePressEvent(mLastMouseEventPosRel.x(),
                                         mLastMouseEventPosRel.y(),
                                         event->timestamp(), 0.5,
                                         mCurrentBrush);
                }
            }
        }
    } else {
        if(event->button() == Qt::LeftButton) {
            handleLeftButtonMousePress();
        } else if(event->button() == Qt::RightButton) {
            handleRightButtonMousePress(event);
        }
    }

    callUpdateSchedulers();
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mCurrentEdge) {
            mCurrentEdge->cancelPassThroughTransform();
        } else {
            cancelSelectedPointsTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
    } else if(mCurrentMode == CanvasMode::MOVE_PATH) {
        if(mRotPivot->isSelected()) {
            mRotPivot->cancelTransform();
        } else {
            cancelSelectedBoxesTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
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
            if(!mLastPressedBox ? true : mLastPressedBox->SWT_isBoxesGroup()) {
                BoundingBox * const pressedBox =
                        getPathAtFromAllAncestors(mCurrentMouseEventPosRel);
                if(!pressedBox) {
                    if(!isShiftPressed()) {
                        clearPointsSelectionOrDeselect();
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentEndPoint();
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
                    clearCurrentEndPoint();
                    clearLastPressedPoint();
                    selectOnlyLastPressedBox();
                }
            }
        }
    } else {
        finishSelectedPointsTransform();
        if(mLastPressedPoint) {
            if(mLastPressedPoint->isCtrlPoint()) {
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
            if(mLastPressedBone) {
                if(mLastPressedBone->isSelected()) {
                    removeBoneFromSelection(mLastPressedBone);
                } else {
                    addBoneToSelection(mLastPressedBone);
                }
            } else {
                if(mLastPressedBox->isSelected()) {
                    removeBoxFromSelection(mLastPressedBox);
                } else {
                    addBoxToSelection(mLastPressedBox);
                }
            }
        } else {
            if(mLastPressedBone) {
                if(!mLastPressedBox->isSelected()) {
                    addBoxToSelection(mLastPressedBox);
                }
                selectOnlyLastPressedBone();
            } else {
                selectOnlyLastPressedBox();
            }
        }
    } else if(mSelecting) {
        moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        finishSelectedBoxesTransform();
    }
}

void Canvas::handleAddPointMousePress() {
    if(mCurrentEndPoint ? mCurrentEndPoint->isHidden() : false) {
        setCurrentEndPoint(nullptr);
    }
    qptr<BoundingBox> test;

    auto nodePointUnderMouse = GetAsPtr(mLastPressedPoint, NodePoint);
    if(nodePointUnderMouse ? !nodePointUnderMouse->isEndPoint() : false) {
        nodePointUnderMouse = nullptr;
    }
    if(nodePointUnderMouse == mCurrentEndPoint &&
            nodePointUnderMouse) return;
    if(!mCurrentEndPoint && !nodePointUnderMouse) {
        const auto newPath = SPtrCreate(VectorPath)();
        mCurrentBoxesGroup->addContainedBox(newPath);
        clearBoxesSelection();
        addBoxToSelection(newPath.get());
        PathAnimator * const newPathAnimator = newPath->getPathAnimator();
        const auto newSinglePath =
                SPtrCreate(VectorPathAnimator)(newPathAnimator);
        setCurrentEndPoint(newSinglePath->
                            addNodeAbsPos(mLastMouseEventPosRel,
                                          mCurrentEndPoint) );
        newPathAnimator->addSinglePathAnimator(newSinglePath);
    } else {
        if(!nodePointUnderMouse) {
            NodePoint * const newPoint =
                    mCurrentEndPoint->addPointAbsPos(mLastMouseEventPosRel);
            //newPoint->startTransform();
            setCurrentEndPoint(newPoint);
        } else if(!mCurrentEndPoint) {
            setCurrentEndPoint(nodePointUnderMouse);
        } else {
            //NodePointUnderMouse->startTransform();
            if(mCurrentEndPoint->getParentPath() ==
               nodePointUnderMouse->getParentPath()) {
                mCurrentEndPoint->connectToPoint(nodePointUnderMouse);
                mCurrentEndPoint->getParentPath()->setPathClosed(true);
            }
            else {
                connectPointsFromDifferentPaths(mCurrentEndPoint,
                                                nodePointUnderMouse);
            }
            setCurrentEndPoint(nodePointUnderMouse);
        }
    } // pats is not null
}

void Canvas::handleAddPointMouseRelease() {
    if(mCurrentEndPoint) {
        if(!mFirstMouseMove) mCurrentEndPoint->finishTransform();
        //mCurrentEndPoint->prp_updateInfluenceRangeAfterChanged();
        if(!mCurrentEndPoint->isEndPoint()) setCurrentEndPoint(nullptr);
    }
}

void Canvas::handleMouseRelease() {
    if(mIsMouseGrabbing) releaseMouseAndDontTrack();
    if(!mDoubleClick) {
        if(mCurrentMode == CanvasMode::MOVE_POINT ||
           mCurrentMode == CanvasMode::ADD_PARTICLE_BOX ||
           mCurrentMode == CanvasMode::ADD_PAINT_BOX ||
           mCurrentMode == CanvasMode::ADD_BONE) {
            handleMovePointMouseRelease();
            if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
                mCanvasWindow->setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
            }/* else if(mCurrentMode == CanvasMode::ADD_BONE) {
                mCanvasWindow->setCanvasMode(CanvasMode::MOVE_POINT);
            }*/
        } else if(isMovingPath()) {
            if(!mLastPressedPoint) {
                handleMovePathMouseRelease();
            } else {
                handleMovePointMouseRelease();
                clearPointsSelection();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseRelease();
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
        }
    }
}

void Canvas::tabletEvent(const QTabletEvent * const e,
                         const QPointF &absPos) {
    if(mCurrentMode != PAINT_MODE || e->buttons() & Qt::MiddleButton) return;
    setLastMouseEventPosAbs(absPos);
    if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton) return;
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = true;

            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mousePressEvent(mLastMouseEventPosRel.x(),
                                         mLastMouseEventPosRel.y(),
                                         e->timestamp(),
                                         e->pressure(),
                                         mCurrentBrush);
                }
            }
        }
    } else if(e->type() == QEvent::TabletRelease) {
        if(e->button() == Qt::LeftButton) {
            mStylusDrawing = false;
            for(const auto& box : mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    const auto paintBox = GetAsPtr(box, PaintBox);
                    paintBox->mouseReleaseEvent();
                }
            }
        }
    } else if(mStylusDrawing) {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                const auto paintBox = GetAsPtr(box, PaintBox);
                paintBox->tabletMoveEvent(mLastMouseEventPosRel.x(),
                                      mLastMouseEventPosRel.y(),
                                      e->timestamp(),
                                      e->pressure(),
                                      e->pointerType() == QTabletEvent::Eraser,
                                      mCurrentBrush);
            }
        }
    } // else if
    callUpdateSchedulers();
}

void Canvas::mouseReleaseEvent(const QMouseEvent * const event) {
    if(event->button() == Qt::LeftButton) schedulePivotUpdate();
    if(isPreviewingOrRendering() || event->button() != Qt::LeftButton) return;
    if(mCurrentMode == PAINT_MODE) {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                const auto paintBox = GetAsPtr(box, PaintBox);
                paintBox->mouseReleaseEvent();
            }
        }
        callUpdateSchedulers();
        return;
    }
    setCurrentMouseEventPosAbs(event->pos());
    mXOnlyTransform = false;
    mYOnlyTransform = false;
    if(mValueInput.inputEnabled()) mFirstMouseMove = false;
    mValueInput.clearAndDisableInput();

    if(mCurrentEdge) {
        if(!mFirstMouseMove) {
            mCurrentEdge->finishPassThroughTransform();
        }
        mHoveredEdge_d = mCurrentEdge;
        mHoveredEdge_d->generatePainterPath();
        mCurrentEdge = nullptr;
    } else {
        handleMouseRelease();
    }
    if(mIsMouseGrabbing) releaseMouseAndDontTrack();
    mLastPressedBone = nullptr;
    mLastPressedBox = nullptr;
    mHoveredPoint_d = mLastPressedPoint;
    mLastPressedPoint = nullptr;

    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
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
    } else if(mCurrentEdge) {
        if(mFirstMouseMove) mCurrentEdge->startPassThroughTransform();
        mCurrentEdge->makePassThroughAbs(mCurrentMouseEventPosRel);
    } else {
        if(mLastPressedPoint) {
            addPointToSelection(mLastPressedPoint);

            if(mLastPressedPoint->isCtrlPoint() ) {
                if(mFirstMouseMove) mLastPressedPoint->startTransform();
                mLastPressedPoint->moveByAbs(
                        getMoveByValueForEventPos(mCurrentMouseEventPosRel));
                return;//
            }/* else {
                mCurrentBoxesGroup->moveSelectedPointsBy(getMoveByValueForEventPos(eventPos),
                                                         mFirstMouseMove);
            }*/
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
            if(mLastPressedBone) {
                addBoneToSelection(mLastPressedBone);
                mLastPressedBone = nullptr;
            }
        }

        if(mSelectedBones.isEmpty()) {
            moveSelectedBoxesByAbs(
                        getMoveByValueForEventPos(
                            mCurrentMouseEventPosRel),
                        mFirstMouseMove);
        } else {
            moveSelectedBonesByAbs(
                        getMoveByValueForEventPos(
                            mCurrentMouseEventPosRel),
                        mFirstMouseMove);
        }
    }
}

void Canvas::handleAddPointMouseMove() {
    if(!mCurrentEndPoint) return;
    if(mFirstMouseMove) mCurrentEndPoint->startTransform();
    if(mCurrentEndPoint->hasNextPoint() &&
       mCurrentEndPoint->hasPreviousPoint()) {
        if(mCurrentEndPoint->getCurrentCtrlsMode() !=
           CtrlsMode::CTRLS_CORNER) {
            mCurrentEndPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);
        }
        if(mCurrentEndPoint->isSeparateNodePoint()) {
            mCurrentEndPoint->moveStartCtrlPtToAbsPos(mLastMouseEventPosRel);
        } else {
            mCurrentEndPoint->moveEndCtrlPtToAbsPos(mLastMouseEventPosRel);
        }
    } else {
        if(!mCurrentEndPoint->hasNextPoint() &&
           !mCurrentEndPoint->hasPreviousPoint()) {
            if(mCurrentEndPoint->getCurrentCtrlsMode() !=
               CtrlsMode::CTRLS_CORNER) {
                mCurrentEndPoint->setCtrlsMode(CtrlsMode::CTRLS_CORNER);
            }
        } else {
            if(mCurrentEndPoint->getCurrentCtrlsMode() !=
               CtrlsMode::CTRLS_SYMMETRIC) {
                mCurrentEndPoint->setCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
            }
        }
        if(mCurrentEndPoint->hasNextPoint()) {
            mCurrentEndPoint->moveStartCtrlPtToAbsPos(mLastMouseEventPosRel);
        } else {
            mCurrentEndPoint->moveEndCtrlPtToAbsPos(mLastMouseEventPosRel);
        }
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
        handleAddPointMouseMove();
    }
}

void Canvas::mouseMoveEvent(const QMouseEvent * const event) {
    if(isPreviewingOrRendering()) return;
    setCurrentMouseEventPosAbs(event->pos());
    if(event->buttons() & Qt::MiddleButton) {
        moveByRel(mCurrentMouseEventPosRel - mLastMouseEventPosRel);
        callUpdateSchedulers();
        return;
    }
    if(mCurrentMode == PAINT_MODE &&
        event->buttons() & Qt::LeftButton)  {
        for(const auto& box : mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                const auto paintBox = GetAsPtr(box, PaintBox);
                paintBox->mouseMoveEvent(mCurrentMouseEventPosRel.x(),
                                         mCurrentMouseEventPosRel.y(),
                                         event->timestamp(),
                                         false, mCurrentBrush);
            }
        }
        callUpdateSchedulers();
        return;
    }
    if((!(event->buttons() & Qt::MiddleButton) &&
       !(event->buttons() & Qt::RightButton) &&
       !(event->buttons() & Qt::LeftButton)) &&
       !mIsMouseGrabbing) {
        const auto lastHoveredPoint = mHoveredPoint_d;
        updateHoveredPoint();

        if(mRotPivot->isVisible() && !mHoveredPoint_d) {
            if(mRotPivot->isPointAtAbsPos(
                        mCurrentMouseEventPosRel,
                        1/mCanvasTransformMatrix.m11()) ) {
                mHoveredPoint_d = mRotPivot.get();
            }
        }

        const auto lastHoveredBox = mHoveredBox;
        updateHoveredBox();

        const auto lastEdge = mHoveredEdge_d;
        clearHoveredEdge();
        if(mCurrentMode == MOVE_POINT) updateHoveredEdge();

        if(mHoveredPoint_d != lastHoveredPoint ||
           mHoveredBox != lastHoveredBox ||
           mHoveredEdge_d != lastEdge) {
            callUpdateSchedulers();
        }
        return;
    }

    if(mMovesToSkip > 0) {
        mMovesToSkip--;
        return;
    }

    if(event->buttons() & Qt::LeftButton || mIsMouseGrabbing) {
        if(mFirstMouseMove && event->buttons() & Qt::LeftButton) {
            if((mCurrentMode == CanvasMode::MOVE_POINT &&
                !mHoveredPoint_d && !mHoveredEdge_d) ||
               (mCurrentMode == CanvasMode::MOVE_PATH &&
                !mHoveredBox && !mHoveredPoint_d)) {
                startSelectionAtPoint(mLastMouseEventPosRel);
            }
        }
        if(mSelecting) {
            moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT ||
                  mCurrentMode == CanvasMode::ADD_PARTICLE_BOX ||
                  mCurrentMode == CanvasMode::ADD_PAINT_BOX ||
                  mCurrentMode == CanvasMode::ADD_BONE) {
            handleMovePointMouseMove();
        } else if(isMovingPath()) {
            if(!mLastPressedPoint) {
                handleMovePathMouseMove();
            } else {
                handleMovePointMouseMove();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
            if(isShiftPressed() ) {
                const qreal lenR = pointToLen(mCurrentMouseEventPosRel -
                                              mLastPressPosRel);
                mCurrentCircle->moveRadiusesByAbs(QPointF(lenR, lenR));
            } else {
                mCurrentCircle->moveRadiusesByAbs(mCurrentMouseEventPosRel -
                                                  mLastPressPosRel);
            }
        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            if(isShiftPressed()) {
                QPointF trans = mCurrentMouseEventPosRel - mLastPressPosRel;
                const qreal valF = qMax(trans.x(), trans.y());
                trans = QPointF(valF, valF);
                mCurrentRectangle->moveSizePointByAbs(trans);
            } else {
                mCurrentRectangle->moveSizePointByAbs(mCurrentMouseEventPosRel -
                                                      mLastPressPosRel);
            }
        }
    }
    mLastMouseEventPosRel = mCurrentMouseEventPosRel;
    mFirstMouseMove = false;
    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

void Canvas::wheelEvent(const QWheelEvent * const event) {
    if(isPreviewingOrRendering()) return;
    if(event->delta() > 0) {
        zoomCanvas(1.1, event->posF());
    } else {
        zoomCanvas(0.9, event->posF());
    }

    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(const QMouseEvent * const e) {
    if(e->modifiers() & Qt::ShiftModifier) return;
    mDoubleClick = true;

    mLastPressedPoint = createNewPointOnLineNearSelected(
                                    mLastPressPosRel,
                                    true,
                                    1/mCanvasTransformMatrix.m11());
    if(!mLastPressedPoint) {
        BoundingBox * const boxAt =
                mCurrentBoxesGroup->getBoxAt(mLastPressPosRel);
        if(!boxAt) {
            if(!mHoveredEdge_d && !mHoveredPoint_d) {
                if(mCurrentBoxesGroup != this) {
                    setCurrentBoxesGroup(mCurrentBoxesGroup->getParentGroup());
                }
            }
        } else {
            if(boxAt->SWT_isBoxesGroup()) {
                setCurrentBoxesGroup(static_cast<BoxesGroup*>(boxAt));
                updateHoveredElements();
            } else if((mCurrentMode == MOVE_PATH ||
                       mCurrentMode == MOVE_POINT) &&
                      boxAt->SWT_isTextBox()) {
                releaseMouseAndDontTrack();
                GetAsPtr(boxAt, TextBox)->openTextEditor(mMainWindow);
            } else if(mCurrentMode == MOVE_PATH &&
                      boxAt->SWT_isVectorPath()) {
                mCanvasWindow->setCanvasMode(MOVE_POINT);
            }
        }
    } else {
        mCurrentEdge = nullptr;
        updateHoveredElements();
    }

    callUpdateSchedulers();
}
