#include "canvas.h"
#include "mainwindow.h"
#include "pathpivot.h"
#include "Boxes/bone.h"

bool Canvas::prp_nextRelFrameWithKey(const int &relFrame,
                                     int &nextRelFrame) {
    int thisNext;
    bool thisHasNext = BoundingBox::prp_nextRelFrameWithKey(relFrame,
                                                            thisNext);
    int minNextFrame = INT_MAX;
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxNext;
        if(box->prp_nextRelFrameWithKey(boxRelFrame, boxNext)) {
            int absNext = box->prp_relFrameToAbsFrame(boxNext);
            if(minNextFrame > absNext) {
                minNextFrame = absNext;
            }
        }
    }
    if(minNextFrame == INT_MAX) {
        if(thisHasNext) {
            nextRelFrame = thisNext;
        }
        return thisHasNext;
    }
    if(thisHasNext) {
        nextRelFrame = qMin(minNextFrame, thisNext);
    } else {
        nextRelFrame = minNextFrame;
    }
    return true;
}

bool Canvas::prp_prevRelFrameWithKey(const int &relFrame,
                                     int &prevRelFrame) {
    int thisPrev;
    bool thisHasPrev = BoundingBox::prp_prevRelFrameWithKey(relFrame,
                                                            thisPrev);
    int minPrevFrame = INT_MIN;
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxPrev;
        if(box->prp_prevRelFrameWithKey(boxRelFrame, boxPrev)) {
            int absPrev = box->prp_relFrameToAbsFrame(boxPrev);
            if(minPrevFrame < absPrev) {
                minPrevFrame = absPrev;
            }
        }
    }
    if(minPrevFrame == INT_MIN) {
        if(thisHasPrev) {
            prevRelFrame = thisPrev;
        }
        return thisHasPrev;
    }
    if(thisHasPrev) {
        prevRelFrame = qMax(minPrevFrame, thisPrev);
    } else {
        prevRelFrame = minPrevFrame;
    }
    return true;
}

void Canvas::shiftAllPointsForAllKeys(const int &by) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            ((VectorPath*)box)->shiftAllPointsForAllKeys(by);
        }
    }}

void Canvas::revertAllPointsForAllKeys() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            ((VectorPath*)box)->revertAllPointsForAllKeys();
        }
    }}

void Canvas::shiftAllPoints(const int &by) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            ((VectorPath*)box)->shiftAllPoints(by);
        }
    }
}

void Canvas::revertAllPoints() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            ((VectorPath*)box)->revertAllPoints();
        }
    }
}

void Canvas::flipSelectedBoxesHorizontally() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(-1., 1.);
        box->finishTransform();
    }
}

void Canvas::flipSelectedBoxesVertically() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(1., -1.);
        box->finishTransform();
    }
}

void Canvas::convertSelectedBoxesToPath() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->objectToVectorPathBox();
    }
}

void Canvas::convertSelectedPathStrokesToPath() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->strokeToVectorPathBox();
    }
}

void Canvas::setSelectedFontFamilyAndStyle(QString family, QString style) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setSelectedFontFamilyAndStyle(family, style);
    }
}

void Canvas::setSelectedFontSize(qreal size) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setSelectedFontSize(size);
    }
}

void Canvas::applyBlurToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new BlurEffect());
    }
}

void Canvas::applyShadowToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ShadowEffect());
    }
}
#include "PixmapEffects/brusheffect.h"
void Canvas::applyBrushEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new BrushEffect());
    }
}

void Canvas::applyLinesEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new LinesEffect());
    }
}

void Canvas::applyCirclesEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new CirclesEffect());
    }
}

void Canvas::applySwirlEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new SwirlEffect());
    }
}

void Canvas::applyOilEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new OilEffect());
    }
}

void Canvas::applyImplodeEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ImplodeEffect());
    }
}

void Canvas::applyDesaturateEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new DesaturateEffect());
    }
}

void Canvas::applyReplaceColorEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ReplaceColorEffect());
    }
}

void Canvas::applyColorizeEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->addEffect(new ColorizeEffect());
    }
}

#include "PathEffects/patheffect.h"
void Canvas::applyDiscretePathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(new DisplacePathEffect(false));
        }
    }
}

void Canvas::applyDuplicatePathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(new DuplicatePathEffect(false));
        }
    }
}

void Canvas::applySolidifyPathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(new SolidifyPathEffect(false));
        }
    }
}

void Canvas::applySumPathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        box->addPathEffect(new SumPathEffect((PathBox*)box, false));
    }
}

void Canvas::applyDiscreteFillPathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(new DisplacePathEffect(false));
        }
    }
}

void Canvas::applyDuplicateFillPathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(new DuplicatePathEffect(false));
        }
    }
}

void Canvas::applySumFillPathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        box->addFillPathEffect(new SumPathEffect((PathBox*)box, false));
    }
}


void Canvas::applyDiscreteOutlinePathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(new DisplacePathEffect(false));
        }
    }
}

void Canvas::applyDuplicateOutlinePathEffectToSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(new DuplicatePathEffect(false));
        }
    }
}

void Canvas::resetSelectedTranslation() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->resetTranslation();
    }
}

void Canvas::resetSelectedScale() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->resetScale();
    }
}

void Canvas::resetSelectedRotation() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->resetRotation();
    }
}

NodePoint *Canvas::createNewPointOnLineNearSelected(
                        const QPointF &absPos,
                        const bool &adjust,
                        const qreal &canvasScaleInv) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        NodePoint *point = box->createNewPointOnLineNear(absPos, adjust,
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
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void Canvas::setSelectedFillColorMode(const ColorMode &mode) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setFillColorMode(mode);
    }
}

void Canvas::setSelectedStrokeColorMode(const ColorMode &mode) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeColorMode(mode);
    }
}

void Canvas::setSelectedCapStyle(Qt::PenCapStyle capStyle) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void Canvas::setSelectedJoinStyle(Qt::PenJoinStyle joinStyle) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void Canvas::setSelectedStrokeWidth(qreal strokeWidth, const bool &finish) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void Canvas::startSelectedStrokeWidthTransform() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void Canvas::startSelectedStrokeColorTransform() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void Canvas::startSelectedFillColorTransform() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

VectorPathEdge *Canvas::getEdgeAt(QPointF absPos) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->isSelected() ) {
            VectorPathEdge *pathEdge = box->getEdge(absPos,
                                                    1./mCanvasTransformMatrix.m11());
            if(pathEdge == NULL) continue;
            return pathEdge;
        }
    }
    return NULL;
}

void Canvas::rotateSelectedBoxesStartAndFinish(const qreal &rotBy) {
    if(mLocalPivot) {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->startRotTransform();
            box->rotateBy(rotBy);
            box->finishTransform();
        }
    } else {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->startRotTransform();
            box->startPosTransform();
            box->saveTransformPivotAbsPos(mRotPivot->getAbsolutePos());
            box->rotateRelativeToSavedPivot(rotBy);
            box->finishTransform();
        }
    }
}

void Canvas::rotateSelectedBy(const qreal &rotBy,
                              const QPointF &absOrigin,
                              const bool &startTrans) {
    if(mSelectedBones.isEmpty()) {
        if(mLocalPivot) {
            if(startTrans) {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->startRotTransform();
                    box->rotateBy(rotBy);
                }
            } else {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->startRotTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->rotateRelativeToSavedPivot(rotBy);
                }
            }
        }
    } else {
        if(mLocalPivot) {
            if(startTrans) {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->startRotTransform();
                    bone->rotateBy(rotBy);
                }
            } else {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->startRotTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->rotateRelativeToSavedPivot(rotBy);
                }
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
    if(mSelectedBones.isEmpty()) {
        if(mLocalPivot) {
            if(startTrans) {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->startScaleTransform();
                    box->scale(scaleXBy, scaleYBy);
                }
            } else {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->startScaleTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->scaleRelativeToSavedPivot(scaleXBy,
                                                   scaleYBy);
                }
            } else {
                Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                    box->scaleRelativeToSavedPivot(scaleXBy,
                                                   scaleYBy);
                }
            }
        }
    } else {
        if(mLocalPivot) {
            if(startTrans) {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->startScaleTransform();
                    bone->scale(scaleXBy, scaleYBy);
                }
            } else {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->startScaleTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->scaleRelativeToSavedPivot(scaleXBy,
                                                    scaleYBy);
                }
            } else {
                Q_FOREACH(Bone *bone, mSelectedBones) {
                    bone->scaleRelativeToSavedPivot(scaleXBy,
                                                    scaleYBy);
                }
            }
        }
    }
}

QPointF Canvas::getSelectedBoxesAbsPivotPos() {
    if(mSelectedBoxes.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    int count = mSelectedBoxes.length();
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

bool Canvas::isSelectionEmpty() {
    return mSelectedBoxes.isEmpty();
}

void Canvas::ungroupSelectedBoxes() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isBoxesGroup()) {
            ((BoxesGroup*) box)->ungroup();
        }
    }
}

void Canvas::centerPivotForSelected() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->centerPivotPosition(true);
    }
}

void Canvas::removeSelectedBoxesAndClearList() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
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

void Canvas::addBoneToSelection(Bone *bone) {
    if(bone->isSelected()) {
        return;
    }
    bone->select();
    mSelectedBones.append(bone); schedulePivotUpdate();
    //sortSelectedBonesByZAscending();
}

void Canvas::removeBoneFromSelection(Bone *bone) {
    bone->deselect();
    mSelectedBones.removeOne(bone); schedulePivotUpdate();
}

void Canvas::clearBonesSelection() {
    Q_FOREACH(Bone *bone, mSelectedBones) {
        bone->deselect();
    }
    mSelectedBones.clear(); schedulePivotUpdate();
}

void Canvas::clearBoxesSelection() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
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
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
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
    Q_FOREACHInverted(box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void Canvas::lowerSelectedBoxesToBottom() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->bringToEnd();
    }
}

void Canvas::lowerSelectedBoxes() {
    BoundingBox *box;
    int lastZ = -10000;
    bool lastBoxChanged = true;
    Q_FOREACHInverted(box, mSelectedBoxes) {
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
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) {
            box->moveUp();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::deselectAllBoxes() {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}
#include "pathpivot.h"
MovablePoint *Canvas::getPointAtAbsPos(const QPointF &absPos,
                                 const CanvasMode &currentMode,
                                 const qreal &canvasScaleInv) {
    if(currentMode == MOVE_POINT ||
       currentMode == ADD_POINT ||
       currentMode == MOVE_PATH ||
       currentMode == ADD_BONE) {
        if(mRotPivot->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mRotPivot;
        }
        MovablePoint *pointAtPos = NULL;
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
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
    if(mSelectedBones.isEmpty()) {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->finishTransform();
        }
    } else {
        Q_FOREACH(Bone *bone, mSelectedBones) {
            bone->finishTransform();
        }
    }
}

void Canvas::cancelSelectedBoxesTransform() {
    if(mSelectedBones.isEmpty()) {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->cancelTransform();
        }
    } else {
        Q_FOREACH(Bone *bone, mSelectedBones) {
            bone->cancelTransform();
        }
    }
}

void Canvas::moveSelectedBoxesByAbs(const QPointF &by,
                                    const bool &startTransform) {
    if(startTransform) {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->startPosTransform();
            box->moveByAbs(by);
        }
    } else {
        Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
            box->moveByAbs(by);
        }
    }
}

void Canvas::moveSelectedBonesByAbs(const QPointF &by,
                                    const bool &startTransform) {
    if(startTransform) {
        Q_FOREACH(Bone *bone, mSelectedBones) {
            bone->startPosTransform();
            bone->moveByAbs(by);
        }
    } else {
        Q_FOREACH(Bone *bone, mSelectedBones) {
            bone->moveByAbs(by);
        }
    }
}


//QPointF BoxesGroup::getRelCenterPosition() {
//    QPointF posSum = QPointF(0., 0.);
//    if(mChildren.isEmpty()) return posSum;
//    int count = mChildren.length();
//    Q_FOREACH(BoundingBox *box, mChildren) {
//        posSum += box->getPivotAbsPos();
//    }
//    return mapAbsPosToRel(posSum/count);
//}

#include "Boxes/linkbox.h"
void Canvas::createLinkBoxForSelected() {
    Q_FOREACH(BoundingBox *selectedBox, mSelectedBoxes) {
        mCurrentBoxesGroup->addContainedBox(selectedBox->createLink());
    }
}

#include "clipboardcontainer.h"
void Canvas::duplicateSelectedBoxes() {
    BoxesClipboardContainer *container =
            new BoxesClipboardContainer();
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    int nBoxes = mSelectedBoxes.count();
    target.write((char*)&nBoxes, sizeof(int));

    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();

    clearBoxesSelection();
    container->pasteTo(mCurrentBoxesGroup);
}

void Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) return;
    BoxesGroup *newGroup = new BoxesGroup();
    mCurrentBoxesGroup->addContainedBox(newGroup);
    BoundingBox *box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        box->removeFromParent();
        newGroup->addContainedBox(box);
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    addBoxToSelection(newGroup);
}

#include "pathoperations.h"
VectorPath *Canvas::getPathResultingFromOperation(
                                const bool &unionInterThis,
                                const bool &unionInterOther) {
    VectorPath *newPath = new VectorPath();
    QList<FullVectorPath*> pathsT;
    FullVectorPath *targetPath = new FullVectorPath();
    pathsT << targetPath;
    FullVectorPath *addToPath = NULL;
    FullVectorPath *addedPath = NULL;

    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox()) {
            SkPath boxPath = ((PathBox*)box)->getRelativePath();
            boxPath.transform(QMatrixToSkMatrix(box->getRelativeTransformAtCurrentFrame()));
            addToPath = targetPath;
            addToPath->generateSinglePathPaths();
            addedPath = new FullVectorPath();
            pathsT << addedPath;
            addedPath->generateFromPath(boxPath);
            addToPath->intersectWith(addedPath,
                                     unionInterThis,
                                     unionInterOther);
            targetPath = new FullVectorPath();
            pathsT << targetPath;
            targetPath->getSeparatePathsFromOther(addToPath);
            targetPath->getSeparatePathsFromOther(addedPath);
        }
    }
    targetPath->generateSinglePathPaths();

    newPath->loadPathFromSkPath(QPainterPathToSkPath(targetPath->getPath()));
    mCurrentBoxesGroup->addContainedBox(newPath);
    foreach(FullVectorPath *pathT, pathsT) {
        delete pathT;
    }
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
#include "Animators/pathanimator.h"
void Canvas::selectedPathsCombine() {
    if(mSelectedBoxes.isEmpty()) return;
    VectorPath *firstVectorPath = NULL;
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            firstVectorPath = (VectorPath*)box;
            break;
        }
    }
    if(firstVectorPath == NULL) {
        firstVectorPath = new VectorPath();
        addContainedBox(firstVectorPath);
    }
    QMatrix firstTranf = firstVectorPath->getCombinedTransform();
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isPathBox()) {
            if(box->SWT_isVectorPath()) {
                if(box == firstVectorPath) continue;
                VectorPath *boxPath = (VectorPath*)box;
                QMatrix relTransf = boxPath->getCombinedTransform()*
                        firstTranf.inverted();
                boxPath->getPathAnimator()->applyTransformToPoints(relTransf);
                boxPath->getPathAnimator()->
                        addAllSinglePathsToAnimator(
                            firstVectorPath->getPathAnimator());
            } else {
                VectorPath *boxPath = ((VectorPath*)box)->objectToVectorPathBox();
                QMatrix relTransf = boxPath->getCombinedTransform()*
                        firstTranf.inverted();
                boxPath->getPathAnimator()->applyTransformToPoints(relTransf);
                boxPath->getPathAnimator()->addAllSinglePathsToAnimator(
                            firstVectorPath->getPathAnimator());
                boxPath->removeFromParent();
            }
        }
    }
}

void Canvas::selectedPathsBreakApart() {
    if(mSelectedBoxes.isEmpty()) return;
    foreach(BoundingBox *box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            ((VectorPath*)box)->breakPathsApart();
        }
    }
}

void Canvas::selectedPathsUnion() {
    VectorPath *newPath = getPathResultingFromOperation(true,
                                                        true);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}
