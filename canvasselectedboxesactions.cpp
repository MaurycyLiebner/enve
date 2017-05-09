#include "canvas.h"
#include "mainwindow.h"

void Canvas::convertSelectedBoxesToPath() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->objectToPath();
    }
}

void Canvas::convertSelectedPathStrokesToPath() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->strokeToPath();
    }
}

void Canvas::setSelectedFontFamilyAndStyle(QString family, QString style) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setSelectedFontFamilyAndStyle(family, style);
    }
}

void Canvas::setSelectedFontSize(qreal size) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setSelectedFontSize(size);
    }
}

void Canvas::applyBlurToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new BlurEffect());
    }
}

void Canvas::applyShadowToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ShadowEffect());
    }
}

void Canvas::applyAlphaMatteToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new AlphaMatteEffect(box));
    }
}

void Canvas::applyBrushEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        //box->addEffect(new BrushEffect());
    }
}

void Canvas::applyLinesEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new LinesEffect());
    }
}

void Canvas::applyCirclesEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new CirclesEffect());
    }
}

void Canvas::applySwirlEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new SwirlEffect());
    }
}

void Canvas::applyOilEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new OilEffect());
    }
}

void Canvas::applyImplodeEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ImplodeEffect());
    }
}

void Canvas::applyDesaturateEffectToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new DesaturateEffect());
    }
}

void Canvas::resetSelectedTranslation() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetTranslation();
    }
}

void Canvas::resetSelectedScale() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetScale();
    }
}

void Canvas::resetSelectedRotation() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->resetRotation();
    }
}

PathPoint *Canvas::createNewPointOnLineNearSelected(const QPointF &absPos,
                                                    const bool &adjust,
                                                    const qreal &canvasScaleInv) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        PathPoint *point = box->createNewPointOnLineNear(absPos, adjust,
                                                         canvasScaleInv);
        if(point != NULL) {
            return point;
        }
    }
    return NULL;
}

void Canvas::setDisplayedFillStrokeSettingsFromLastSelected() {
    if(mSelectedBoxes.isEmpty()) {
        mFillStrokeSettingsWidget->setCurrentSettings(NULL,
                                                      NULL);
    } else {
        setCurrentFillStrokeSettingsFromBox(mSelectedBoxes.last() );
    }
}


void Canvas::applyPaintSettingToSelected(
        const PaintSetting &setting) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void Canvas::setSelectedFillColorMode(const ColorMode &mode) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillColorMode(mode);
    }
}

void Canvas::setSelectedStrokeColorMode(const ColorMode &mode) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeColorMode(mode);
    }
}

void Canvas::setSelectedFillGradient(Gradient *gradient, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillGradient(gradient, finish);
    }
}

void Canvas::setSelectedStrokeGradient(Gradient *gradient, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeGradient(gradient, finish);
    }
}

void Canvas::setSelectedFillFlatColor(Color color, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setFillFlatColor(color, finish);
    }
}

void Canvas::setSelectedStrokeFlatColor(Color color, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeFlatColor(color, finish);
    }
}

void Canvas::setSelectedCapStyle(Qt::PenCapStyle capStyle) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void Canvas::setSelectedJoinStyle(Qt::PenJoinStyle joinStyle) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void Canvas::setSelectedStrokeWidth(qreal strokeWidth, bool finish) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void Canvas::startSelectedStrokeWidthTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void Canvas::startSelectedStrokeColorTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void Canvas::startSelectedFillColorTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

VectorPathEdge *Canvas::getEdgeAt(QPointF absPos) {
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->isSelected() ) {
            VectorPathEdge *pathEdge = box->getEgde(absPos,
                                                    1./mCanvasTransformMatrix.m11());
            if(pathEdge == NULL) continue;
            return pathEdge;
        }
    }
    return NULL;
}

void Canvas::rotateSelectedBy(const qreal &rotBy,
                              const QPointF &absOrigin,
                              const bool &startTrans) {
    if(mLocalPivot || !mGlobalPivotVisible) {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startRotTransform();
                box->rotateBy(rotBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->rotateBy(rotBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startRotTransform();
                box->startPosTransform();
                box->saveTransformPivotAbsPos(absOrigin);
                box->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->rotateRelativeToSavedPivot(rotBy);
            }
        }
    }
}

void Canvas::scaleSelectedBy(qreal scaleBy,
                             QPointF absOrigin,
                             bool startTrans) {
    scaleSelectedBy(scaleBy, scaleBy,
                    absOrigin, startTrans);
}

void Canvas::scaleSelectedBy(qreal scaleXBy, qreal scaleYBy,
                                 QPointF absOrigin,
                                 bool startTrans) {
    if(mLocalPivot || !mGlobalPivotVisible) {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->scale(scaleXBy, scaleYBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scale(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->startScaleTransform();
                box->startPosTransform();
                box->saveTransformPivotAbsPos(absOrigin);
                box->scaleRelativeToSavedPivot(scaleXBy,
                                               scaleYBy);
            }
        } else {
            foreach(BoundingBox *box, mSelectedBoxes) {
                box->scaleRelativeToSavedPivot(scaleXBy,
                                               scaleYBy);
            }
        }
    }
}

QPointF Canvas::getSelectedBoxesAbsPivotPos() {
    if(mSelectedBoxes.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    int count = mSelectedBoxes.length();
    foreach(BoundingBox *box, mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

bool Canvas::isSelectionEmpty() {
    return mSelectedBoxes.isEmpty();
}

void Canvas::ungroupSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->isGroup()) {
            ((BoxesGroup*) box)->ungroup();
        }
    }
}

void Canvas::centerPivotForSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->centerPivotPosition(true);
    }
}

void Canvas::removeSelectedBoxesAndClearList() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        //box->deselect();
        box->removeFromParent();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void Canvas::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) {
        return;
    }
    box->select();
    mSelectedBoxes.append(box); schedulePivotUpdate();
    sortSelectedBoxesByZAscending();
    //setCurrentFillStrokeSettingsFromBox(box);
    mMainWindow->setCurrentBox(box);
}

void Canvas::removeBoxFromSelection(BoundingBox *box) {
    box->deselect();
    mSelectedBoxes.removeOne(box); schedulePivotUpdate();
    if(mSelectedBoxes.isEmpty()) {
        mMainWindow->setCurrentBox(NULL);
    } else {
        mMainWindow->setCurrentBox(mSelectedBoxes.last());
    }
}

void Canvas::clearBoxesSelection() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    mMainWindow->setCurrentBox(NULL);
//    if(mLastPressedBox != NULL) {
//        mLastPressedBox->deselect();
//        mLastPressedBox = NULL;
//    }
}


void Canvas::applyCurrentTransformationToSelected() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->applyCurrentTransformation();
    }
}

bool zLessThan(BoundingBox *box1, BoundingBox *box2) {
    return box1->getZIndex() > box2->getZIndex();
}

void Canvas::sortSelectedBoxesByZAscending() {
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
}

void Canvas::raiseSelectedBoxesToTop() {
    BoundingBox *box;
    foreachInverted(box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void Canvas::lowerSelectedBoxesToBottom() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->bringToEnd();
    }
}

void Canvas::lowerSelectedBoxes() {
    BoundingBox *box;
    int lastZ = -10000;
    bool lastBoxChanged = true;
    foreachInverted(box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ - 1 != lastZ || lastBoxChanged) {
            box->moveDown();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::raiseSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    foreach(BoundingBox *box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) {
            box->moveUp();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::deselectAllBoxes() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}
#include "pathpivot.h"
MovablePoint *Canvas::getPointAtAbsPos(const QPointF &absPos,
                                 const CanvasMode &currentMode,
                                 const qreal &canvasScaleInv) {
    if(currentMode == MOVE_POINT ||
       currentMode == ADD_POINT ||
       currentMode == MOVE_PATH) {
        if(mRotPivot->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mRotPivot;
        }
        MovablePoint *pointAtPos = NULL;
        foreach(BoundingBox *box, mSelectedBoxes) {
            pointAtPos = box->getPointAtAbsPos(absPos,
                                               currentMode,
                                               canvasScaleInv);
            if(pointAtPos != NULL) {
                break;
            }
        }
        return pointAtPos;
    }
    return NULL;
}

void Canvas::finishSelectedBoxesTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->finishTransform();
    }
}

void Canvas::cancelSelectedBoxesTransform() {
    foreach(BoundingBox *box, mSelectedBoxes) {
        box->cancelTransform();
    }
}

void Canvas::moveSelectedBoxesByAbs(const QPointF &by,
                                    const bool &startTransform) {
    if(startTransform) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->startPosTransform();
            box->moveByAbs(by);
        }
    } else {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->moveByAbs(by);
        }
    }
}

//QPointF BoxesGroup::getRelCenterPosition() {
//    QPointF posSum = QPointF(0., 0.);
//    if(mChildren.isEmpty()) return posSum;
//    int count = mChildren.length();
//    foreach(BoundingBox *box, mChildren) {
//        posSum += box->getPivotAbsPos();
//    }
//    return mapAbsPosToRel(posSum/count);
//}

#include "Boxes/linkbox.h"
void Canvas::createLinkBoxForSelected() {
    foreach(BoundingBox *selectedBox, mSelectedBoxes) {
        selectedBox->createLink(mCurrentBoxesGroup);
    }
}

void Canvas::duplicateSelectedBoxes() {
    foreach(BoundingBox *selectedBox, mSelectedBoxes) {
        selectedBox->createDuplicate();
    }
}

BoxesGroup* Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) {
        return NULL;
    }
    BoxesGroup *newGroup = new BoxesGroup(mCurrentBoxesGroup);
    BoundingBox *box;
    foreachInverted(box, mSelectedBoxes) {
        box->removeFromParent();
        newGroup->addChild(box);
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    return newGroup;
}

#include "pathoperations.h"
VectorPath *Canvas::getPathResultingFromOperation(
                                const bool &unionInterThis,
                                const bool &unionInterOther) {
    VectorPath *newPath = new VectorPath(mCurrentBoxesGroup);
    FullVectorPath *targetPath = new FullVectorPath();
    FullVectorPath *addToPath = NULL;
    FullVectorPath *addedPath = NULL;

    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->isVectorPath() ||
           box->isCircle() ||
           box->isRect() ||
           box->isText()) {
            const QPainterPath &boxPath = box->getRelativeTransform().map(
                                        ((PathBox*)box)->getRelativePath());
            addToPath = targetPath;
            addToPath->generateSinglePathPaths();
            addedPath = new FullVectorPath();
            addedPath->generateFromPath(boxPath);
            addToPath->intersectWith(addedPath,
                                     unionInterThis,
                                     unionInterOther);
            targetPath = new FullVectorPath();
            targetPath->getSeparatePathsFromOther(addToPath);
            targetPath->getSeparatePathsFromOther(addedPath);

            delete addedPath;
            delete addToPath;
        }
    }

    targetPath->addAllToVectorPath(newPath->getPathAnimator());

    return newPath;
}

void Canvas::selectedPathsDifference() {
    VectorPath *newPath = getPathResultingFromOperation(false,
                                                        true);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsIntersection() {
    VectorPath *newPath = getPathResultingFromOperation(false,
                                                        false);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsDivision() {
    VectorPath *newPath1 = getPathResultingFromOperation(false,
                                                        false);

    VectorPath *newPath2 = getPathResultingFromOperation(false,
                                                        true);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}

void Canvas::selectedPathsExclusion() {
    VectorPath *newPath1 = getPathResultingFromOperation(false,
                                                         true);
    VectorPath *newPath2 = getPathResultingFromOperation(true,
                                                         false);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}

void Canvas::selectedPathsUnion() {
    VectorPath *newPath = getPathResultingFromOperation(true,
                                                        true);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}
