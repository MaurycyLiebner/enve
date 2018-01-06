#include "canvas.h"
#include <QMouseEvent>
#include <QMenu>
#include "pathpivot.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/imagebox.h"
#include "Boxes/textbox.h"
#include "Boxes/linkbox.h"
#include "edge.h"
#include "nodepoint.h"
#include "Animators/pathanimator.h"
#include "pointhelpers.h"
#include "Boxes/particlebox.h"
#include "clipboardcontainer.h"
#include "mainwindow.h"
#include "Boxes/paintbox.h"
#include "Paint/PaintLib/brush.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "fontswidget.h"
#include "Boxes/bone.h"

void Canvas::handleMovePathMousePressEvent() {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPosRel);
    mLastPressedBone = NULL;
    if(mLastPressedBox == NULL) {
        if(!isShiftPressed() ) {
            clearBoxesSelection();
        }
    } else {
        if(mLastPressedBox->SWT_isBonesBox()) {
            if(mBonesSelectionEnabled) {
                mLastPressedBone = ((BonesBox*)mLastPressedBox)->
                        getBoneAtAbsPos(mLastMouseEventPosRel);
                if(!isShiftPressed()) {
                    clearBonesSelection();
                }
                return;
            }
        }
        if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
            clearBoxesSelection();
        }
    }
}

void Canvas::addCanvasActionToMenu(QMenu *menu) {
    bool hasVectorPathBox = false;
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            hasVectorPathBox = true;
            break;
        }
    }
    bool hasGroups = false;
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isBoxesGroup()) {
            hasGroups = true;
            break;
        }
    }
    bool hasPathBox = false;
    if(hasVectorPathBox) {
        hasPathBox = true;
    } else {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPathBox()) {
                hasPathBox = true;
                break;
            }
        }
    }
    if(hasVectorPathBox || hasGroups) {
        menu->addAction("Apply Transformation")->setObjectName(
                    "canvas_apply_transformation");
    }
    menu->addSeparator();
    menu->addAction("Create Link")->setObjectName(
                "canvas_create_link");
    menu->addAction("Center Pivot")->setObjectName(
                "canvas_center_pivot");
    menu->addSeparator();
    QAction *copyAction = menu->addAction("Copy");
    copyAction->setObjectName("canvas_copy");
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);
    QAction *cutAction = menu->addAction("Cut");
    cutAction->setObjectName("canvas_cut");
    cutAction->setShortcut(Qt::CTRL + Qt::Key_X);
    QAction *duplicateAction = menu->addAction("Duplicate");
    duplicateAction->setObjectName("canvas_duplicate");
    duplicateAction->setShortcut(Qt::CTRL + Qt::Key_D);
    QAction *deleteAction = menu->addAction("Delete");
    deleteAction->setObjectName("canvas_delete");
    deleteAction->setShortcut(Qt::Key_Delete);
    menu->addSeparator();
    QAction *groupAction = menu->addAction("Group");
    groupAction->setObjectName("canvas_group");
    groupAction->setShortcut(Qt::CTRL + Qt::Key_G);

    QAction *ungroupAction = menu->addAction("Ungroup");
    ungroupAction->setEnabled(hasGroups);
    ungroupAction->setObjectName("canvas_ungroup");
    ungroupAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    menu->addSeparator();

    QMenu *effectsMenu = menu->addMenu("Effects");
    effectsMenu->addAction("Blur")->setObjectName(
                "canvas_effects_blur");
    effectsMenu->addAction("Shadow")->setObjectName(
                "canvas_effects_shadow");
//    effectsMenu->addAction("Lines");
//    effectsMenu->addAction("Circles");
//    effectsMenu->addAction("Swirl");
//    effectsMenu->addAction("Oil");
//    effectsMenu->addAction("Implode");
    effectsMenu->addAction("Desaturate")->setObjectName(
                "canvas_effects_desaturate");
    effectsMenu->addAction("Colorize")->setObjectName(
                "canvas_effects_colorize");
    effectsMenu->addAction("Replace Color")->setObjectName(
                "canvas_effects_replace_color");
    effectsMenu->addAction("Brush")->setObjectName(
                "canvas_effects_brush");

    if(hasPathBox || hasGroups) {
        QMenu *pathEffectsMenu = menu->addMenu("Path Effects");
        pathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                    "canvas_path_effects_discrete");
        pathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                    "canvas_path_effects_duplicate");
        pathEffectsMenu->addAction("Solidify Effect")->setObjectName(
                    "canvas_path_effects_solidify");
        pathEffectsMenu->addAction("Operation Effect")->setObjectName(
                    "canvas_path_effect_sum");
        if(hasGroups) {
            pathEffectsMenu->addAction("Group Sum Effect")->setObjectName(
                        "canvas_path_effect_group_sum");
        }

        QMenu *fillPathEffectsMenu = menu->addMenu("Fill Effects");
        fillPathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                    "canvas_fill_effects_discrete");
        fillPathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                    "canvas_fill_effects_duplicate");
        fillPathEffectsMenu->addAction("Operation Effect")->setObjectName(
                    "canvas_fill_effect_sum");

        QMenu *outlinePathEffectsMenu = menu->addMenu("Outline Effects");
        outlinePathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                    "canvas_outline_effects_discrete");
        outlinePathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                    "canvas_outline_effects_duplicate");
        outlinePathEffectsMenu->addAction("Operation Effect")->setObjectName(
                    "canvas_outline_effect_sum");
    }

    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPaintBox()) {
            menu->addSeparator();
            menu->addAction("New Paint Frame")->setObjectName(
                        "canvas_new_paint_frame");
            menu->addAction("New Empty Paint Frame")->setObjectName(
                        "canvas_new_empty_paint_frame");
            menu->addAction("Setup Animation Frames")->setObjectName(
                        "canvas_setup_animation_frames");
            menu->addSeparator();
            menu->addAction("Load From Image")->setObjectName(
                        "canvas_load_from_img");
            break;
        }
    }
}
#include <QFileDialog>
#include "Paint/paintboxsettingsdialog.h"
#include "customfpsdialog.h"
#include "PixmapEffects/brusheffect.h"
bool Canvas::handleSelectedCanvasAction(QAction *selectedAction) {
    if(selectedAction->objectName() == "canvas_copy") {
        copyAction();
    } if(selectedAction->objectName() == "canvas_cut") {
        cutAction();
    } if(selectedAction->objectName() == "canvas_duplicate") {
        duplicateSelectedBoxes();
    } else if(selectedAction->objectName() == "canvas_delete") {
        removeSelectedBoxesAndClearList();
    } else if(selectedAction->objectName() == "canvas_apply_transformation") {
        applyCurrentTransformationToSelected();
    } else if(selectedAction->objectName() == "canvas_create_link") {
        createLinkBoxForSelected();
    } else if(selectedAction->objectName() == "canvas_group") {
        groupSelectedBoxes();
    } else if(selectedAction->objectName() == "canvas_ungroup") {
        ungroupSelectedBoxes();
    } else if(selectedAction->objectName() == "canvas_center_pivot") {
        centerPivotForSelected();
    } else if(selectedAction->objectName() == "canvas_effects_blur") {
        applyBlurToSelected();
    } else if(selectedAction->objectName() == "canvas_effects_shadow") {
        applyShadowToSelected();
    } else if(selectedAction->objectName() == "canvas_effects_desaturate") {
        applyDesaturateEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_effects_colorize") {
        applyColorizeEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_effects_brush") {
        applyBrushEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_effects_replace_color") {
        applyReplaceColorEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_path_effects_discrete") {
        applyDiscretePathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_path_effects_duplicate") {
        applyDuplicatePathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_path_effects_solidify") {
        applySolidifyPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_path_effect_sum") {
        applySumPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_path_effect_group_sum") {
        applyGroupSumPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_fill_effects_discrete") {
        applyDiscreteFillPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_fill_effects_duplicate") {
        applyDuplicateFillPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_fill_effect_sum") {
        applySumFillPathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_outline_effects_discrete") {
        applyDiscreteOutlinePathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_outline_effects_duplicate") {
        applyDuplicateOutlinePathEffectToSelected();
    } else if(selectedAction->objectName() == "canvas_new_paint_frame") {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
                paintBox->newPaintFrameOnCurrentFrame();
            }
        }
    } else if(selectedAction->objectName() == "canvas_new_empty_paint_frame") {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
                paintBox->newEmptyPaintFrameOnCurrentFrame();
            }
        }
    } else if(selectedAction->objectName() == "canvas_setup_animation_frames") {
        PaintBoxSettingsDialog dialog;
        PaintBox *firstPaintBox = (PaintBox*)mSelectedBoxes.first();
        int frameStep = firstPaintBox->getFrameStep();
        int overlapFrames = firstPaintBox->getOverlapFrames();
        dialog.setOverlapFrames(overlapFrames);
        dialog.setFrameStep(frameStep);
        dialog.exec();
        if(dialog.result() == QDialog::Rejected) return true;
        frameStep = dialog.getFrameStep();
        overlapFrames = dialog.getOverlapFrames();
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
                paintBox->setOverlapFrames(overlapFrames);
                paintBox->setFrameStep(frameStep);
            }
        }
    } else if(selectedAction->objectName() == "canvas_load_from_img") {
        MainWindow::getInstance()->disableEventFilter();
        QString importPath = QFileDialog::getOpenFileName(
                                                MainWindow::getInstance(),
                                                "Load From Image", "",
                                                "Image Files (*.png *.jpg)");
        MainWindow::getInstance()->enableEventFilter();
        if(!importPath.isEmpty()) {
            QImage img;
            if(img.load(importPath)) {
                foreach(BoundingBox *box, mSelectedBoxes) {
                    if(box->SWT_isPaintBox()) {
                        PaintBox *paintBox = (PaintBox*)box;
                        paintBox->loadFromImage(img);
                    }
                }
            }
        }
    } else {
        return false;
    }
    return true;
}
#include "PathEffects/patheffect.h"
void Canvas::handleRightButtonMousePress(QMouseEvent *event) {
    if(mIsMouseGrabbing) {
        cancelCurrentTransform();
        mValueInput.clearAndDisableInput();
    } else {
        QPointF eventPos = mapCanvasAbsToRel(event->pos());
        BoundingBox *pressedBox = mCurrentBoxesGroup->getBoxAt(eventPos);
        if(pressedBox == NULL) {
            clearBoxesSelection();

            QMenu menu(mCanvasWindow->getCanvasWidget());

            BoxesClipboardContainer *clipboard =
                    (BoxesClipboardContainer*)
                    MainWindow::getInstance()->getClipboardContainer(
                                                CCT_BOXES);
            if(clipboard != NULL) {
                menu.addAction("Paste")->setShortcut(Qt::CTRL + Qt::Key_V);
            }

            const QList<CanvasQSPtr> &listOfCanvas =
                    mCanvasWindow->getCanvasList();
            QMenu *linkCanvasMenu;
            if(listOfCanvas.count() > 1) {
                linkCanvasMenu = menu.addMenu("Link Canvas");
                Q_FOREACH(const CanvasQSPtr &canvas, listOfCanvas) {
                    QAction *action =
                            linkCanvasMenu->addAction(canvas->getName());
                    if(canvas == this) {
                        action->setEnabled(false);
                        action->setVisible(false);
                    }
                }
            }

            QMenu *effectsMenu = menu.addMenu("Effects");
            effectsMenu->addAction("Blur");
            effectsMenu->addAction("Brush");
            effectsMenu->addAction("Lines");
            effectsMenu->addAction("Circles");
            effectsMenu->addAction("Swirl");
            effectsMenu->addAction("Oil");
            effectsMenu->addAction("Implode");
            effectsMenu->addAction("Desaturate");
            QMenu *pathEffectsMenu = menu.addMenu("Path Effects");
            pathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                        "canvas_path_effects_discrete");
            pathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                        "canvas_path_effects_duplicate");
            pathEffectsMenu->addAction("Solidify Effect")->setObjectName(
                        "canvas_path_effects_solidify");

            QMenu *fillPathEffectsMenu = menu.addMenu("Fill Effects");
            fillPathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                        "canvas_fill_effects_discrete");
            fillPathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                        "canvas_fill_effects_duplicate");
            fillPathEffectsMenu->addAction("Operation Effect")->setObjectName(
                        "canvas_fill_effect_sum");

            QMenu *outlinePathEffectsMenu = menu.addMenu("Outline Effects");
            outlinePathEffectsMenu->addAction("Discrete Effect")->setObjectName(
                        "canvas_outline_effects_discrete");
            outlinePathEffectsMenu->addAction("Duplicate Effect")->setObjectName(
                        "canvas_outline_effects_duplicate");
            outlinePathEffectsMenu->addAction("Operation Effect")->setObjectName(
                        "canvas_outline_effect_sum");


            menu.addAction("Settings...");

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction != NULL) {
                if(selectedAction->text() == "Paste") {
                    pasteAction();
                } else if(selectedAction->text()== "Settings...") {
                    mCanvasWindow->openSettingsWindowForCurrentCanvas();
                } else if(selectedAction->text() == "Blur") {
                    addEffect(new BlurEffect());
                } else if(selectedAction->text() == "Brush") {
                    addEffect(new BrushEffect());
                } else if(selectedAction->text() == "Lines") {
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
                } else if(selectedAction->objectName() == "canvas_path_effects_discrete") {
                    addPathEffect(new DisplacePathEffect(false));
                } else if(selectedAction->objectName() == "canvas_path_effects_duplicate") {
                    addPathEffect(new DuplicatePathEffect(false));
                } else if(selectedAction->objectName() == "canvas_path_effects_solidify") {
                    addPathEffect(new SolidifyPathEffect(false));
                } else if(selectedAction->objectName() == "canvas_fill_effects_discrete") {
                    addFillPathEffect(new DisplacePathEffect(false));
                } else if(selectedAction->objectName() == "canvas_fill_effects_duplicate") {
                    addFillPathEffect(new DuplicatePathEffect(false));
                } else if(selectedAction->objectName() == "canvas_outline_effects_discrete") {
                    addOutlinePathEffect(new DisplacePathEffect(true));
                } else if(selectedAction->objectName() == "canvas_outline_effects_duplicate") {
                    addOutlinePathEffect(new DuplicatePathEffect(true));
                } else { // link canvas
                    const QList<QAction*> &canvasActions =
                            linkCanvasMenu->actions();
                    int id = canvasActions.indexOf(selectedAction);
                    if(id >= 0) {
                        QSharedPointer<InternalLinkCanvas> canvasLink =
                                listOfCanvas.at(id)->
                                createLink()->ref<InternalLinkCanvas>();
                        mCurrentBoxesGroup->addContainedBox(canvasLink.data());
                        canvasLink->centerPivotPosition();
                    }
                }
            } else {

            }
        } else {
            if(!pressedBox->isSelected()) {
                if(!isShiftPressed()) {
                    clearBoxesSelection();
                }
                addBoxToSelection(pressedBox);
            }

            QMenu menu(mCanvasWindow->getCanvasWidget());

            pressedBox->addActionsToMenu(&menu);
            addCanvasActionToMenu(&menu);

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction) {
                if(!handleSelectedCanvasAction(selectedAction)) {
                    pressedBox->handleSelectedCanvasAction(selectedAction);
                }
            } else {

            }
        }
    }
}

void Canvas::clearHoveredEdge() {
    if(mHoveredEdge == NULL) return;
    mHoveredEdge = NULL;
}

void Canvas::handleMovePointMousePressEvent() {
    if(mLastPressedPoint == NULL) {
        if(isCtrlPressed() ) {
            clearPointsSelection();
            mLastPressedPoint = createNewPointOnLineNearSelected(
                                        mLastPressPosRel,
                                        isShiftPressed(),
                                        1./mCanvasTransformMatrix.m11());

        } else {
            mCurrentEdge = getEdgeAt(mLastPressPosRel);
            if(mCurrentEdge != NULL) {
                clearPointsSelection();
                clearCurrentEndPoint();
                clearLastPressedPoint();
            }
        }
        clearHoveredEdge();
    } else {
        if(mLastPressedPoint->isSelected()) {
            return;
        }
        if(!isShiftPressed() &&
           !(mLastPressedPoint->isCtrlPoint())) {
            clearPointsSelection();
        }
        if(mLastPressedPoint->isCtrlPoint() ) {
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
                                   1./mCanvasTransformMatrix.m11());

    if(mRotPivot->handleMousePress(mLastMouseEventPosRel,
                                   1./mCanvasTransformMatrix.m11())) {
    } else if(isMovingPath()) {
        if(mHoveredPoint == NULL) {
            handleMovePathMousePressEvent();
        } else {
            handleMovePointMousePressEvent();
        }
    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mLastPressedBox = getPathAtFromAllAncestors(mLastPressPosRel);
    } else {
        if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMousePress();
        } // point adding mode
        else if (mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMousePressEvent();
        } else if (mCurrentMode == CanvasMode::DRAW_PATH) {
            handleDrawPathMousePressEvent();
        } else if (mCurrentMode == CanvasMode::ADD_DRAW_PATH_NODE) {
            handleAddDrawPathNodeMousePressEvent();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {

            QSharedPointer<Circle> newPath =
                    (new Circle())->ref<Circle>();
            mCurrentBoxesGroup->addContainedBox(newPath.data());
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);
            //newPath->startAllPointsTransform();
            clearBoxesSelection();
            addBoxToSelection(newPath.data());

            mCurrentCircle = newPath.data();

        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            QSharedPointer<Rectangle> newPath =
                    (new Rectangle())->ref<Rectangle>();
            mCurrentBoxesGroup->addContainedBox(newPath.data());
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);
            //newPath->startAllPointsTransform();
            clearBoxesSelection();
            addBoxToSelection(newPath.data());

            mCurrentRectangle = newPath.data();
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
            QSharedPointer<TextBox> newPath = (new TextBox())->ref<TextBox>();
            FontsWidget *fonstWidget = mMainWindow->getFontsWidget();
            newPath->setSelectedFontFamilyAndStyle(
                        fonstWidget->getCurrentFontFamily(),
                        fonstWidget->getCurrentFontStyle());
            newPath->setSelectedFontSize(fonstWidget->getCurrentFontSize());
            mCurrentBoxesGroup->addContainedBox(newPath.data());
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);

            mCurrentTextBox = newPath.data();

            clearBoxesSelection();
            addBoxToSelection(newPath.data());
        } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
            //setCanvasMode(CanvasMode::MOVE_POINT);
            QSharedPointer<ParticleBox> partBox =
                    (new ParticleBox())->ref<ParticleBox>();
            mCurrentBoxesGroup->addContainedBox(partBox.data());
            partBox->setAbsolutePos(mLastMouseEventPosRel, false);
            clearBoxesSelection();
            addBoxToSelection(partBox.data());

            mLastPressedPoint = partBox->getBottomRightPoint();
        } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_EMITTER) {
            Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                if(box->SWT_isParticleBox()) {
                    QPointF relPos = box->mapAbsPosToRel(mLastMouseEventPosRel);
                    if(box->getRelBoundingRectPath().contains(relPos.x(),
                                                              relPos.y())) {
                        ((ParticleBox*)box)->addEmitterAtAbsPos(
                                    mLastMouseEventPosRel);
                        break;
                    }
                }
            }
        } else if(mCurrentMode == CanvasMode::ADD_PAINT_BOX) {
            //setCanvasMode(CanvasMode::MOVE_POINT);
            QSharedPointer<PaintBox> paintBox = (new PaintBox())->ref<PaintBox>();
            mCurrentBoxesGroup->addContainedBox(paintBox.data());
            paintBox->setAbsolutePos(mLastMouseEventPosRel, false);
            clearBoxesSelection();
            clearPointsSelection();
            addBoxToSelection(paintBox.data());

            mLastPressedPoint = paintBox->getBottomRightPoint();
            addPointToSelection(mLastPressedPoint);
            mLastPressedPoint->startTransform();
        } else if(mCurrentMode == CanvasMode::ADD_BONE) {
            //setCanvasMode(CanvasMode::MOVE_POINT);
            BonePt *bonePt = NULL;

            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isBonePoint()) {
                    bonePt = (BonePt*)mLastPressedPoint;
                    if(bonePt->getTipBone() == NULL) {
                        mLastMouseEventPosRel = bonePt->getAbsolutePos();
                        bonePt = NULL;
                    }
                }
            }
            Bone *newBone = NULL;
            if(bonePt == NULL) {
                QSharedPointer<BonesBox> boneBox;
                if(mSelectedBoxes.count() > 0) {
                    BoundingBox *lastSelected = mSelectedBoxes.last();
                    if(lastSelected->SWT_isBonesBox()) {
                        boneBox = ((BonesBox*)lastSelected)->ref<BonesBox>();

                        newBone = new Bone(boneBox.data());

                        newBone->getRootPt()->setAbsolutePos(
                                    mLastMouseEventPosRel);
                        newBone->getTipPt()->setAbsolutePos(
                                    mLastMouseEventPosRel);
                    }
                }
                if(boneBox == NULL) {
                    boneBox = (new BonesBox())->ref<BonesBox>();
                    mCurrentBoxesGroup->addContainedBox(boneBox.data());
                    boneBox->setAbsolutePos(mLastMouseEventPosRel, false);
                    clearBoxesSelection();
                    addBoxToSelection(boneBox.data());
                    newBone = new Bone(boneBox.data());
                }
            } else {
                Bone *boneT = bonePt->getTipBone();
                newBone = new Bone(boneT);
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

void Canvas::mousePressEvent(QMouseEvent *event) {
    if(isPreviewingOrRendering()) return;
    setLastMouseEventPosAbs(event->pos());
    if(mCurrentMode == PAINT_MODE) {
        if(mStylusDrawing) {
            return;
        }
        if(event->button() == Qt::LeftButton) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    PaintBox *paintBox = (PaintBox*)box;
                    paintBox->mousePressEvent(mLastMouseEventPosRel.x(),
                                         mLastMouseEventPosRel.y(),
                                         event->timestamp(), 0.5,
                                         mCurrentBrush);
                }
            }
        }
        callUpdateSchedulers();
        return;
    }
    if(event->button() != Qt::LeftButton) {
        if(event->button() == Qt::RightButton) {
            handleRightButtonMousePress(event);
        }
    } else {
        handleLeftButtonMousePress();
    }

    callUpdateSchedulers();
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mCurrentEdge != NULL) {
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

    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mCanvasWindow->setCanvasMode(MOVE_PATH);
    }

    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
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
        if(mLastPressedPoint != NULL) {
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
            if((mLastPressedBox == NULL) ? true : mLastPressedBox->SWT_isBoxesGroup()) {
                BoundingBox *pressedBox = getPathAtFromAllAncestors(
                            mCurrentMouseEventPosRel);
                if(pressedBox == NULL) {
                    if(!(isShiftPressed()) ) {
                        clearPointsSelectionOrDeselect();
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentEndPoint();
                    clearLastPressedPoint();
                    setCurrentBoxesGroup((BoxesGroup*) pressedBox->getParentGroup());
                    addBoxToSelection(pressedBox);
                    mLastPressedBox = pressedBox;
                }
            }
            if(mLastPressedBox != NULL) {
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
        if(mLastPressedPoint != NULL) {
            if(mLastPressedPoint->isCtrlPoint() ) {
                removePointFromSelection(mLastPressedPoint);
            }
        }
    }
}


void Canvas::handleMovePathMouseRelease() {
    if(mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        finishSelectedBoxesTransform();
    } else if(mFirstMouseMove) {
        mSelecting = false;
        if(isShiftPressed() && mLastPressedBox != NULL) {
            if(mLastPressedBone != NULL) {
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
            if(mLastPressedBone != NULL) {
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
    if(mCurrentEndPoint != NULL) {
        if(mCurrentEndPoint->isHidden()) {
            setCurrentEndPoint(NULL);
        }
    }
    NodePoint *nodePointUnderMouse = (NodePoint*) mLastPressedPoint;
    if( (nodePointUnderMouse == NULL) ? false :
            !nodePointUnderMouse->isEndPoint() ) {
        nodePointUnderMouse = NULL;
    }
    if(nodePointUnderMouse == mCurrentEndPoint &&
            nodePointUnderMouse != NULL) {
        return;
    }
    if(mCurrentEndPoint == NULL && nodePointUnderMouse == NULL) {
        QSharedPointer<VectorPath> newPath =
                (new VectorPath())->ref<VectorPath>();
        mCurrentBoxesGroup->addContainedBox(newPath.data());
        clearBoxesSelection();
        addBoxToSelection(newPath.data());
        PathAnimator *newPathAnimator = newPath->getPathAnimator();
        VectorPathAnimator *newSinglePath = new VectorPathAnimator(
                                                    newPathAnimator);
        setCurrentEndPoint(newSinglePath->
                            addNodeAbsPos(mLastMouseEventPosRel,
                                            mCurrentEndPoint) );
        newPathAnimator->addSinglePathAnimator(newSinglePath);
    } else {
        if(nodePointUnderMouse == NULL) {
            NodePoint *newPoint =
                    mCurrentEndPoint->addPointAbsPos(mLastMouseEventPosRel);
            //newPoint->startTransform();
            setCurrentEndPoint(newPoint);
        } else if(mCurrentEndPoint == NULL) {
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

void Canvas::handleDrawPathMousePressEvent() {
    if(isShiftPressed()) return;
    mDrawPath.startDrawingPath(mLastPressPosRel, mLastPressPosRel);
}

void Canvas::handleDrawPathMouseMoveEvent() {
    if(isShiftPressed()) return;
    mDrawPath.drawPathMove(mLastMouseEventPosRel);
}

void Canvas::handleDrawPathMouseReleaseEvent() {
    if(isShiftPressed()) {
        mDrawPath.addNodeAt(mLastMouseEventPosRel);
    } else {
        mDrawPath.finishDrawingPath(false);
    }
}

void Canvas::handleAddDrawPathNodeMousePressEvent() {

}

void Canvas::handleAddPointMouseRelease() {
    if(mCurrentEndPoint != NULL) {
        //mCurrentEndPoint->prp_updateInfluenceRangeAfterChanged();
        if(!mCurrentEndPoint->isEndPoint()) {
            setCurrentEndPoint(NULL);
        }
    }
}

void Canvas::handleMouseRelease() {
    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
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
            if(mLastPressedPoint == NULL) {
                handleMovePathMouseRelease();
            } else {
                handleMovePointMouseRelease();
                clearPointsSelection();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseRelease();
        } else if(mCurrentMode == CanvasMode::DRAW_PATH) {
            handleDrawPathMouseReleaseEvent();
        } else if(mCurrentMode == PICK_PATH_SETTINGS) {
            if(mLastPressedBox != NULL) {
                PathBox *srcPathBox = (PathBox*) mLastPressedBox;
                foreach(BoundingBox *box, mSelectedBoxes) {
                    if(box->SWT_isPathBox()) {
                        PathBox *pathBox = (PathBox*)box;
                        if(mPickStrokeFromPath) {
                            pathBox->duplicateStrokeSettingsFrom(
                                        srcPathBox->getStrokeSettings());
                            pathBox->resetStrokeGradientPointsPos();
                        }
                        if(mPickFillFromPath) {
                            pathBox->duplicateFillSettingsFrom(
                                        srcPathBox->getFillSettings());
                            pathBox->resetFillGradientPointsPos();
                        }
                    }
                }
            }
            mCanvasWindow->setCanvasMode(MOVE_PATH);
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
            if(mCurrentTextBox != NULL) {
                mCurrentTextBox->openTextEditor();
            }
        }
    }
}

void Canvas::tabletEvent(QTabletEvent *e,
                         const QPointF &absPos) {
    if(mCurrentMode != PAINT_MODE) return;
    setLastMouseEventPosAbs(absPos);
    if(e->type() == QEvent::TabletPress) {
        if(e->button() == Qt::RightButton) {
            return;
        } else if(e->button() == Qt::LeftButton) {
            mStylusDrawing = true;

            foreach(BoundingBox *box, mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    PaintBox *paintBox = (PaintBox*)box;
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
            foreach(BoundingBox *box, mSelectedBoxes) {
                if(box->SWT_isPaintBox()) {
                    PaintBox *paintBox = (PaintBox*)box;
                    paintBox->mouseReleaseEvent();
                }
            }
        }
    } else if(mStylusDrawing) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
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

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        schedulePivotUpdate();
    }
    if(isPreviewingOrRendering() ||
            event->button() != Qt::LeftButton) return;
    if(mCurrentMode == PAINT_MODE) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
                paintBox->mouseReleaseEvent();
            }
        }
        callUpdateSchedulers();
        return;
    }
    setCurrentMouseEventPosAbs(event->pos());
    mXOnlyTransform = false;
    mYOnlyTransform = false;
    if(mValueInput.inputEnabled()) {
        mFirstMouseMove = false;
    }
    mValueInput.clearAndDisableInput();

    if(mCurrentEdge != NULL) {
        if(!mFirstMouseMove) {
            mCurrentEdge->finishPassThroughTransform();
        }
        mHoveredEdge = mCurrentEdge;
        mHoveredEdge->generatePainterPath();
        mCurrentEdge = NULL;
    } else {
        handleMouseRelease();
    }
    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
    mLastPressedBone = NULL;
    mLastPressedBox = NULL;
    mHoveredPoint = mLastPressedPoint;
    mLastPressedPoint = NULL;

    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

QPointF Canvas::getMoveByValueForEventPos(const QPointF &eventPos) {
    QPointF moveByPoint = eventPos - mLastPressPosRel;
    if(mValueInput.inputEnabled()) {
        moveByPoint = QPointF(mValueInput.getValue(),
                              mValueInput.getValue());
    }
    if(mYOnlyTransform) {
        moveByPoint.setX(0.);
    } else if(mXOnlyTransform) {
        moveByPoint.setY(0.);
    }
    return moveByPoint;
}

void Canvas::handleMovePointMouseMove() {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }
        mRotPivot->moveByAbs(getMoveByValueForEventPos(mCurrentMouseEventPosRel));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
           mRotPivot->handleMouseMove(mCurrentMouseEventPosRel,
                                      mLastPressPosRel,
                                      mXOnlyTransform, mYOnlyTransform,
                                      mValueInput.inputEnabled(),
                                      mValueInput.getValue(),
                                      mFirstMouseMove,
                                      mCurrentMode);
    } else if(mCurrentEdge != NULL) {
        if(mFirstMouseMove) {
            mCurrentEdge->startPassThroughTransform();
        }
        mCurrentEdge->makePassThrough(mCurrentMouseEventPosRel);
    } else {
        if(mLastPressedPoint != NULL) {
            addPointToSelection(mLastPressedPoint);

            if(mLastPressedPoint->isCtrlPoint() ) {
                if(mFirstMouseMove) {
                    mLastPressedPoint->startTransform();
                }
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
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }

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
        if(mLastPressedBox != NULL) {
            addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
            if(mLastPressedBone != NULL) {
                addBoneToSelection(mLastPressedBone);
                mLastPressedBone = NULL;
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
    if(mCurrentEndPoint == NULL) return;
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
        if(mLastPressedPoint == NULL) {
            handleMovePathMouseMove();
        } else {
            handleMovePointMouseMove();
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddPointMouseMove();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if(isPreviewingOrRendering()) return;
    setCurrentMouseEventPosAbs(event->pos());
    if(mCurrentMode == PAINT_MODE &&
       event->buttons() & Qt::LeftButton)  {
        foreach(BoundingBox *box, mSelectedBoxes) {
            if(box->SWT_isPaintBox()) {
                PaintBox *paintBox = (PaintBox*)box;
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
        MovablePoint *lastHoveredPoint = mHoveredPoint;
        updateHoveredPoint();

        if(mRotPivot->isVisible() && mHoveredPoint == NULL) {
            if(mRotPivot->isPointAtAbsPos(
                        mCurrentMouseEventPosRel,
                        1./mCanvasTransformMatrix.m11()) ) {
                mHoveredPoint = mRotPivot;
            }
        }

        BoundingBox *lastHoveredBox = mHoveredBox;
        updateHoveredBox();

        VectorPathEdge *lastEdge = mHoveredEdge;
        clearHoveredEdge();
        if(mCurrentMode == MOVE_POINT) {
            updateHoveredEdge();
        }

        if(mHoveredPoint != lastHoveredPoint ||
           mHoveredBox != lastHoveredBox ||
           mHoveredEdge != lastEdge) {
            callUpdateSchedulers();
        }
        return;
    }

    if(mMovesToSkip > 0) {
        mMovesToSkip--;
        return;
    }

    if(event->buttons() & Qt::MiddleButton) {
        moveByRel(mCurrentMouseEventPosRel - mLastMouseEventPosRel);
    } else if(event->buttons() & Qt::LeftButton ||
               mIsMouseGrabbing) {
        if(mFirstMouseMove && event->buttons() & Qt::LeftButton) {
            if((mCurrentMode == CanvasMode::MOVE_POINT &&
                mHoveredPoint == NULL &&
                mHoveredEdge == NULL) ||
               (mCurrentMode == CanvasMode::MOVE_PATH &&
                mHoveredBox == NULL && mHoveredPoint == NULL)) {
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
            if(mLastPressedPoint == NULL) {
                handleMovePathMouseMove();
            } else {
                handleMovePointMouseMove();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove();
        } else if(mCurrentMode == CanvasMode::DRAW_PATH) {
            handleDrawPathMouseMoveEvent();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
            if(isShiftPressed() ) {
                qreal lenR = pointToLen(mCurrentMouseEventPosRel -
                                        mLastPressPosRel);
                mCurrentCircle->moveRadiusesByAbs(QPointF(lenR, lenR));
            } else {
                mCurrentCircle->moveRadiusesByAbs(mCurrentMouseEventPosRel -
                                                  mLastPressPosRel);
            }
        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            if(isShiftPressed()) {
                QPointF trans = mCurrentMouseEventPosRel - mLastPressPosRel;
                qreal valF = qMax(trans.x(), trans.y() );
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

void Canvas::wheelEvent(QWheelEvent *event) {
    if(isPreviewingOrRendering()) return;
    if(event->delta() > 0) {
        zoomCanvas(1.1, event->posF());
    } else {
        zoomCanvas(0.9, event->posF());
    }
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
    
    if(mHoveredEdge != NULL) {
        mHoveredEdge->generatePainterPath();
    }

    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *e) {
    if(e->modifiers() & Qt::ShiftModifier) return;
    mDoubleClick = true;

    mLastPressedPoint = createNewPointOnLineNearSelected(
                                    mLastPressPosRel,
                                    true,
                                    1./mCanvasTransformMatrix.m11());
    if(mLastPressedPoint == NULL) {
        BoundingBox *boxAt = mCurrentBoxesGroup->getBoxAt(mLastPressPosRel);
        if(boxAt == NULL) {
            if(mHoveredEdge == NULL && mHoveredPoint == NULL) {
                if(mCurrentBoxesGroup != this) {
                    setCurrentBoxesGroup((BoxesGroup*)
                                         mCurrentBoxesGroup->getParentGroup());
                }
            }
        } else {
            if(boxAt->SWT_isBoxesGroup()) {
                setCurrentBoxesGroup((BoxesGroup*) boxAt);
                updateHoveredElements();
            } else if((mCurrentMode == MOVE_PATH ||
                       mCurrentMode == MOVE_POINT) &&
                      boxAt->SWT_isTextBox()) {
                releaseMouseAndDontTrack();
                ((TextBox*) boxAt)->openTextEditor();
            } else if(mCurrentMode == MOVE_PATH &&
                      boxAt->SWT_isVectorPath()) {
                mCanvasWindow->setCanvasMode(MOVE_POINT);
            }
        }
    } else {
        mCurrentEdge = NULL;
        updateHoveredElements();
    }

    callUpdateSchedulers();
}
