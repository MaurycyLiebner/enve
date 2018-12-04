#include "canvas.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/bone.h"
#include "MovablePoints/pathpivot.h"

#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"

bool Canvas::prp_nextRelFrameWithKey(const int &relFrame,
                                     int &nextRelFrame) {
    int thisNext;
    bool thisHasNext = BoundingBox::prp_nextRelFrameWithKey(relFrame,
                                                            thisNext);
    int minNextFrame = INT_MAX;
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->shiftAllPointsForAllKeys(by);
        }
    }}

void Canvas::revertAllPointsForAllKeys() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->revertAllPointsForAllKeys();
        }
    }}

void Canvas::shiftAllPoints(const int &by) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->shiftAllPoints(by);
        }
    }
}

void Canvas::revertAllPoints() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->revertAllPoints();
        }
    }
}

void Canvas::flipSelectedBoxesHorizontally() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(-1., 1.);
        box->finishTransform();
    }
}

void Canvas::flipSelectedBoxesVertically() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(1., -1.);
        box->finishTransform();
    }
}

void Canvas::convertSelectedBoxesToPath() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->objectToVectorPathBox();
    }
}

void Canvas::convertSelectedPathStrokesToPath() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->strokeToVectorPathBox();
    }
}

void Canvas::setSelectedFontFamilyAndStyle(
        const QString& family, const QString& style) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setSelectedFontFamilyAndStyle(family, style);
    }
}

void Canvas::setSelectedFontSize(const qreal &size) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setSelectedFontSize(size);
    }
}

void Canvas::applySampledMotionBlurToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->addEffect(SPtrCreate(SampledMotionBlurEffect)(box));
    }
}

#include "PathEffects/patheffect.h"
void Canvas::applyDiscretePathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(DisplacePathEffect)(false));
        }
    }
}

void Canvas::applyDuplicatePathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(DuplicatePathEffect)(false));
        }
    }
}

void Canvas::applyLengthPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(LengthPathEffect)(false));
        }
    }
}

void Canvas::applySolidifyPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(SolidifyPathEffect)(false));
        }
    }
}

void Canvas::applySumPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        auto pathBox = GetAsPtr(box, PathBox);
        box->addPathEffect(SPtrCreate(OperationPathEffect)(pathBox, false));
    }
}

void Canvas::applyGroupSumPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(!box->SWT_isBoxesGroup()) continue;
        auto groupBox = GetAsPtr(box, BoxesGroup);
        box->addPathEffect(SPtrCreate(GroupLastPathSumPathEffect)(groupBox, false));
    }
}

void Canvas::applyDiscreteFillPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(SPtrCreate(DisplacePathEffect)(false));
        }
    }
}

void Canvas::applyDuplicateFillPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(SPtrCreate(DuplicatePathEffect)(false));
        }
    }
}

void Canvas::applySumFillPathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        auto pathBox = GetAsPtr(box, PathBox);
        box->addFillPathEffect(SPtrCreate(OperationPathEffect)(pathBox, false));
    }
}


void Canvas::applyDiscreteOutlinePathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(SPtrCreate(DisplacePathEffect)(true));
        }
    }
}

void Canvas::applyDuplicateOutlinePathEffectToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(SPtrCreate(DuplicatePathEffect)(true));
        }
    }
}

void Canvas::resetSelectedTranslation() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->resetTranslation();
    }
}

void Canvas::resetSelectedScale() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->resetScale();
    }
}

void Canvas::resetSelectedRotation() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->resetRotation();
    }
}

NodePoint* Canvas::createNewPointOnLineNearSelected(
                        const QPointF &absPos,
                        const bool &adjust,
                        const qreal &canvasScaleInv) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        NodePoint *point = box->createNewPointOnLineNear(absPos, adjust,
                                                         canvasScaleInv);
        if(point != nullptr) {
            return point;
        }
    }
    return nullptr;
}

void Canvas::getDisplayedFillStrokeSettingsFromLastSelected(
        PaintSettings*& fillSetings, StrokeSettings*& strokeSettings) {
    if(mSelectedBoxes.isEmpty()) {
        fillSetings = nullptr;
        strokeSettings = nullptr;
    } else {
        auto box = mSelectedBoxes.last();
        fillSetings = box->getFillSettings();
        strokeSettings = box->getStrokeSettings();
    }
}


void Canvas::applyPaintSettingToSelected(PaintSetting *setting) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void Canvas::setSelectedFillColorMode(const ColorMode &mode) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setFillColorMode(mode);
    }
}

void Canvas::setSelectedStrokeColorMode(const ColorMode &mode) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setStrokeColorMode(mode);
    }
}

void Canvas::setSelectedCapStyle(const Qt::PenCapStyle &capStyle) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void Canvas::setSelectedJoinStyle(const Qt::PenJoinStyle& joinStyle) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void Canvas::setSelectedStrokeWidth(const qreal &strokeWidth, const bool &finish) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth, finish);
    }
}

void Canvas::startSelectedStrokeWidthTransform() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void Canvas::startSelectedStrokeColorTransform() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void Canvas::startSelectedFillColorTransform() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

VectorPathEdge *Canvas::getEdgeAt(QPointF absPos) {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->isSelected() ) {
            VectorPathEdge *pathEdge = box->getEdge(absPos,
                                                    1./mCanvasTransformMatrix.m11());
            if(pathEdge == nullptr) continue;
            return pathEdge;
        }
    }
    return nullptr;
}

void Canvas::rotateSelectedBoxesStartAndFinish(const qreal &rotBy) {
    if(mLocalPivot) {
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
            box->startRotTransform();
            box->rotateBy(rotBy);
            box->finishTransform();
        }
    } else {
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->startRotTransform();
                    box->rotateBy(rotBy);
                }
            } else {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->startRotTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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

void Canvas::scaleSelectedBy(const qreal &scaleBy,
                             const QPointF &absOrigin,
                             const bool &startTrans) {
    scaleSelectedBy(scaleBy, scaleBy,
                    absOrigin, startTrans);
}

void Canvas::scaleSelectedBy(const qreal& scaleXBy,
                             const qreal& scaleYBy,
                             const QPointF& absOrigin,
                             const bool& startTrans) {
    if(mSelectedBones.isEmpty()) {
        if(mLocalPivot) {
            if(startTrans) {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->startScaleTransform();
                    box->scale(scaleXBy, scaleYBy);
                }
            } else {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
                    box->startScaleTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->scaleRelativeToSavedPivot(scaleXBy,
                                                   scaleYBy);
                }
            } else {
                Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

bool Canvas::isSelectionEmpty() {
    return mSelectedBoxes.isEmpty();
}

void Canvas::ungroupSelectedBoxes() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isBoxesGroup()) {
            GetAsPtr(box, BoxesGroup)->ungroup();
        }
    }
}

void Canvas::centerPivotForSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->centerPivotPosition(true);
    }
}

void Canvas::removeSelectedBoxesAndClearList() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
        mMainWindow->setCurrentBox(nullptr);
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
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    mMainWindow->setCurrentBox(nullptr);
//    if(mLastPressedBox != nullptr) {
//        mLastPressedBox->deselect();
//        mLastPressedBox = nullptr;
//    }
}


void Canvas::applyCurrentTransformationToSelected() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->applyCurrentTransformation();
    }
}

bool zLessThan(const qptr<BoundingBox>& box1, const qptr<BoundingBox>& box2) {
    return box1->getZIndex() > box2->getZIndex();
}

void Canvas::sortSelectedBoxesByZAscending() {
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
}

void Canvas::raiseSelectedBoxesToTop() {
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void Canvas::lowerSelectedBoxesToBottom() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->bringToEnd();
    }
}

void Canvas::lowerSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    BoundingBox* box;
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
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) {
            box->moveUp();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::deselectAllBoxes() {
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}

MovablePoint *Canvas::getPointAtAbsPos(const QPointF &absPos,
                                 const CanvasMode &currentMode,
                                 const qreal &canvasScaleInv) {
    if(currentMode == MOVE_POINT ||
       currentMode == ADD_POINT ||
       currentMode == MOVE_PATH ||
       currentMode == ADD_BONE) {
        if(mRotPivot->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mRotPivot.get();
        }
        MovablePoint *pointAtPos = nullptr;
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
            pointAtPos = box->getPointAtAbsPos(absPos,
                                               currentMode,
                                               canvasScaleInv);
            if(pointAtPos != nullptr) {
                break;
            }
        }
        return pointAtPos;
    }
    return nullptr;
}

void Canvas::finishSelectedBoxesTransform() {
    if(mSelectedBones.isEmpty()) {
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
            box->startPosTransform();
            box->moveByAbs(by);
        }
    } else {
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
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
//    Q_FOREACH(const qsptr<BoundingBox>& box, mChildren) {
//        posSum += box->getPivotAbsPos();
//    }
//    return mapAbsPosToRel(posSum/count);
//}

#include "Boxes/linkbox.h"
void Canvas::createLinkBoxForSelected() {
    Q_FOREACH(const qptr<BoundingBox>& selectedBox, mSelectedBoxes) {
        mCurrentBoxesGroup->addContainedBox(selectedBox->createLink());
    }
}

#include "clipboardcontainer.h"
void Canvas::duplicateSelectedBoxes() {
    stdsptr<BoxesClipboardContainer> container =
            SPtrCreate(BoxesClipboardContainer)();
    QBuffer target(container->getBytesArray());
    target.open(QIODevice::WriteOnly);
    int nBoxes = mSelectedBoxes.count();
    target.write(reinterpret_cast<char*>(&nBoxes), sizeof(int));

    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();

    clearBoxesSelection();
    container->pasteTo(mCurrentBoxesGroup);
}

void Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) return;
    qsptr<BoxesGroup> newGroup = SPtrCreate(BoxesGroup)();
    mCurrentBoxesGroup->addContainedBox(newGroup);
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        qsptr<BoundingBox> boxSP = GetAsSPtr(box, BoundingBox);
        box->removeFromParent();
        newGroup->addContainedBox(boxSP);
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    addBoxToSelection(newGroup.get());
}

#include "pathoperations.h"
VectorPath *Canvas::getPathResultingFromOperation(
                                const bool &unionInterThis,
                                const bool &unionInterOther) {
    qsptr<VectorPath> newPath = SPtrCreate(VectorPath)();
    QList<FullVectorPath*> pathsT;
    FullVectorPath *targetPath = new FullVectorPath();
    pathsT << targetPath;
    FullVectorPath *addToPath = nullptr;
    FullVectorPath *addedPath = nullptr;

    Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox()) {
            SkPath boxPath = GetAsPtr(box, PathBox)->getRelativePath();
            QMatrix boxTrans = box->getRelativeTransformAtCurrentFrame();
            boxPath.transform(QMatrixToSkMatrix(boxTrans));
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
    return newPath.get();
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
    VectorPath *firstVectorPath = nullptr;
    foreach(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            firstVectorPath = GetAsPtr(box, VectorPath);
            break;
        }
    }
    if(firstVectorPath == nullptr) {
        auto newPath = SPtrCreate(VectorPath)();
        addContainedBox(newPath);
        firstVectorPath = newPath.get();
    }
    QMatrix firstTranf = firstVectorPath->getCombinedTransform();
    foreach(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isPathBox()) {
            if(box->SWT_isVectorPath()) {
                if(box == firstVectorPath) continue;
                VectorPath *boxPath = GetAsPtr(box, VectorPath);
                QMatrix relTransf = boxPath->getCombinedTransform()*
                        firstTranf.inverted();
                boxPath->getPathAnimator()->applyTransformToPoints(relTransf);
                boxPath->getPathAnimator()->
                        addAllSinglePathsToAnimator(
                            firstVectorPath->getPathAnimator());
            } else {
                VectorPath *boxPath = GetAsPtr(box, VectorPath)->objectToVectorPathBox();
                QMatrix relTransf = boxPath->getCombinedTransform()*
                        firstTranf.inverted();
                boxPath->getPathAnimator()->applyTransformToPoints(relTransf);
                boxPath->getPathAnimator()->addAllSinglePathsToAnimator(
                            firstVectorPath->getPathAnimator());
            }
            box->removeFromParent();
        }
    }
}

void Canvas::selectedPathsBreakApart() {
    if(mSelectedBoxes.isEmpty()) return;
    foreach(const qptr<BoundingBox>& box, mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->breakPathsApart();
        }
    }
}

void Canvas::selectedPathsUnion() {
    VectorPath *newPath = getPathResultingFromOperation(true,
                                                        true);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}
