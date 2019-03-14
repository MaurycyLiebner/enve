#include "canvas.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/bone.h"
#include "MovablePoints/pathpivot.h"
#include "Boxes/vectorpath.h"
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"

bool Canvas::anim_nextRelFrameWithKey(const int &relFrame,
                                     int &nextRelFrame) {
    int thisNext;
    bool thisHasNext = BoundingBox::anim_nextRelFrameWithKey(relFrame,
                                                            thisNext);
    int minNextFrame = FrameRange::EMAX;
    for(const auto &box : mSelectedBoxes) {
        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxNext;
        if(box->anim_nextRelFrameWithKey(boxRelFrame, boxNext)) {
            int absNext = box->prp_relFrameToAbsFrame(boxNext);
            if(minNextFrame > absNext) {
                minNextFrame = absNext;
            }
        }
    }
    if(minNextFrame == FrameRange::EMAX) {
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

bool Canvas::anim_prevRelFrameWithKey(const int &relFrame,
                                     int &prevRelFrame) {
    int thisPrev;
    bool thisHasPrev = BoundingBox::anim_prevRelFrameWithKey(relFrame,
                                                            thisPrev);
    int minPrevFrame = FrameRange::EMIN;
    for(const auto &box : mSelectedBoxes) {
        int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxPrev;
        if(box->anim_prevRelFrameWithKey(boxRelFrame, boxPrev)) {
            int absPrev = box->prp_relFrameToAbsFrame(boxPrev);
            if(minPrevFrame < absPrev) {
                minPrevFrame = absPrev;
            }
        }
    }
    if(minPrevFrame == FrameRange::EMIN) {
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
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->shiftAllPointsForAllKeys(by);
        }
    }}

void Canvas::revertAllPointsForAllKeys() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->revertAllPointsForAllKeys();
        }
    }}

void Canvas::shiftAllPoints(const int &by) {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->shiftAllPoints(by);
        }
    }
}

void Canvas::revertAllPoints() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->revertAllPoints();
        }
    }
}

void Canvas::flipSelectedBoxesHorizontally() {
    for(const auto &box : mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(-1., 1.);
        box->finishTransform();
    }
}

void Canvas::flipSelectedBoxesVertically() {
    for(const auto &box : mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(1., -1.);
        box->finishTransform();
    }
}

void Canvas::convertSelectedBoxesToPath() {
    for(const auto &box : mSelectedBoxes) {
        box->objectToVectorPathBox();
    }
}

void Canvas::convertSelectedPathStrokesToPath() {
    for(const auto &box : mSelectedBoxes) {
        box->strokeToVectorPathBox();
    }
}

void Canvas::setSelectedFontFamilyAndStyle(
        const QString& family, const QString& style) {
    for(const auto &box : mSelectedBoxes) {
        box->setSelectedFontFamilyAndStyle(family, style);
    }
}

void Canvas::setSelectedFontSize(const qreal &size) {
    for(const auto &box : mSelectedBoxes) {
        box->setSelectedFontSize(size);
    }
}
#include "GPUEffects/gpurastereffect.h"
void Canvas::applyGPURasterEffectToSelected(
        const stdsptr<GPURasterEffectCreator>& creator) {
    for(const auto &box : mSelectedBoxes) {
        box->addGPUEffect(GetAsSPtr(creator->create(), GPURasterEffect));
    }
}

void Canvas::applySampledMotionBlurToSelected() {
    for(const auto &box : mSelectedBoxes) {
        box->addEffect(SPtrCreate(SampledMotionBlurEffect)(box));
    }
}

#include "PathEffects/patheffect.h"
void Canvas::applyDiscretePathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(DisplacePathEffect)(false));
        }
    }
}

void Canvas::applyDuplicatePathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(DuplicatePathEffect)(false));
        }
    }
}

void Canvas::applyLengthPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(LengthPathEffect)(false));
        }
    }
}

void Canvas::applySolidifyPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addPathEffect(SPtrCreate(SolidifyPathEffect)(false));
        }
    }
}

void Canvas::applySumPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        auto pathBox = GetAsPtr(box, PathBox);
        box->addPathEffect(SPtrCreate(OperationPathEffect)(pathBox, false));
    }
}

void Canvas::applyGroupSumPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(!box->SWT_isBoxesGroup()) continue;
        auto groupBox = GetAsPtr(box, BoxesGroup);
        box->addPathEffect(SPtrCreate(GroupLastPathSumPathEffect)(groupBox, false));
    }
}

void Canvas::applyDiscreteFillPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(SPtrCreate(DisplacePathEffect)(false));
        }
    }
}

void Canvas::applyDuplicateFillPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addFillPathEffect(SPtrCreate(DuplicatePathEffect)(false));
        }
    }
}

void Canvas::applySumFillPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        auto pathBox = GetAsPtr(box, PathBox);
        box->addFillPathEffect(SPtrCreate(OperationPathEffect)(pathBox, false));
    }
}


void Canvas::applyDiscreteOutlinePathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(SPtrCreate(DisplacePathEffect)(true));
        }
    }
}

void Canvas::applyDuplicateOutlinePathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox() || box->SWT_isBoxesGroup()) {
            box->addOutlinePathEffect(SPtrCreate(DuplicatePathEffect)(true));
        }
    }
}

void Canvas::resetSelectedTranslation() {
    for(const auto &box : mSelectedBoxes) {
        box->resetTranslation();
    }
}

void Canvas::resetSelectedScale() {
    for(const auto &box : mSelectedBoxes) {
        box->resetScale();
    }
}

void Canvas::resetSelectedRotation() {
    for(const auto &box : mSelectedBoxes) {
        box->resetRotation();
    }
}

NodePoint* Canvas::createNewPointOnLineNearSelected(
                        const QPointF &absPos,
                        const bool &adjust,
                        const qreal &canvasScaleInv) {
    for(const auto &box : mSelectedBoxes) {
        NodePoint *point = box->createNewPointOnLineNear(absPos, adjust,
                                                         canvasScaleInv);
        if(point) {
            return point;
        }
    }
    return nullptr;
}

void Canvas::getDisplayedFillStrokeSettingsFromLastSelected(
        PaintSettingsAnimator*& fillSetings, OutlineSettingsAnimator*& strokeSettings) {
    if(mSelectedBoxes.isEmpty()) {
        fillSetings = nullptr;
        strokeSettings = nullptr;
    } else {
        auto box = mSelectedBoxes.last();
        fillSetings = box->getFillSettings();
        strokeSettings = box->getStrokeSettings();
    }
}


void Canvas::applyPaintSettingToSelected(const PaintSettingsApplier &setting) {
    for(const auto &box : mSelectedBoxes) {
        box->applyPaintSetting(setting);
    }
}

void Canvas::setSelectedFillColorMode(const ColorMode &mode) {
    for(const auto &box : mSelectedBoxes) {
        box->setFillColorMode(mode);
    }
}

void Canvas::setSelectedStrokeColorMode(const ColorMode &mode) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeColorMode(mode);
    }
}

void Canvas::setSelectedCapStyle(const Qt::PenCapStyle &capStyle) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void Canvas::setSelectedJoinStyle(const Qt::PenJoinStyle& joinStyle) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void Canvas::setSelectedStrokeWidth(const qreal &strokeWidth) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeWidth(strokeWidth);
    }
}

void Canvas::setSelectedStrokeBrush(SimpleBrushWrapper * const brush) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeBrush(brush);
    }
}

void Canvas::setSelectedStrokeBrushWidthCurve(
        const qCubicSegment1D& curve) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeBrushWidthCurve(curve);
    }
}


void Canvas::setSelectedStrokeBrushTimeCurve(
        const qCubicSegment1D& curve) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeBrushTimeCurve(curve);
    }
}


void Canvas::setSelectedStrokeBrushPressureCurve(
        const qCubicSegment1D& curve) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeBrushPressureCurve(curve);
    }
}

void Canvas::startSelectedStrokeWidthTransform() {
    for(const auto &box : mSelectedBoxes) {
        box->startSelectedStrokeWidthTransform();
    }
}

void Canvas::startSelectedStrokeColorTransform() {
    for(const auto &box : mSelectedBoxes) {
        box->startSelectedStrokeColorTransform();
    }
}

void Canvas::startSelectedFillColorTransform() {
    for(const auto &box : mSelectedBoxes) {
        box->startSelectedFillColorTransform();
    }
}

VectorPathEdge *Canvas::getEdgeAt(QPointF absPos) {
    for(const auto &box : mSelectedBoxes) {
        if(box->isSelected() ) {
            VectorPathEdge *pathEdge = box->getEdge(absPos,
                                                    1./mCanvasTransformMatrix.m11());
            if(!pathEdge) continue;
            return pathEdge;
        }
    }
    return nullptr;
}

void Canvas::rotateSelectedBoxesStartAndFinish(const qreal &rotBy) {
    if(mLocalPivot) {
        for(const auto &box : mSelectedBoxes) {
            box->startRotTransform();
            box->rotateBy(rotBy);
            box->finishTransform();
        }
    } else {
        for(const auto &box : mSelectedBoxes) {
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
                for(const auto &box : mSelectedBoxes) {
                    box->startRotTransform();
                    box->rotateBy(rotBy);
                }
            } else {
                for(const auto &box : mSelectedBoxes) {
                    box->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                for(const auto &box : mSelectedBoxes) {
                    box->startRotTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                for(const auto &box : mSelectedBoxes) {
                    box->rotateRelativeToSavedPivot(rotBy);
                }
            }
        }
    } else {
        if(mLocalPivot) {
            if(startTrans) {
                for(Bone *bone : mSelectedBones) {
                    bone->startRotTransform();
                    bone->rotateBy(rotBy);
                }
            } else {
                for(Bone *bone : mSelectedBones) {
                    bone->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                for(Bone *bone : mSelectedBones) {
                    bone->startRotTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                for(Bone *bone : mSelectedBones) {
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
                for(const auto &box : mSelectedBoxes) {
                    box->startScaleTransform();
                    box->scale(scaleXBy, scaleYBy);
                }
            } else {
                for(const auto &box : mSelectedBoxes) {
                    box->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                for(const auto &box : mSelectedBoxes) {
                    box->startScaleTransform();
                    box->startPosTransform();
                    box->saveTransformPivotAbsPos(absOrigin);
                    box->scaleRelativeToSavedPivot(scaleXBy,
                                                   scaleYBy);
                }
            } else {
                for(const auto &box : mSelectedBoxes) {
                    box->scaleRelativeToSavedPivot(scaleXBy,
                                                   scaleYBy);
                }
            }
        }
    } else {
        if(mLocalPivot) {
            if(startTrans) {
                for(Bone *bone : mSelectedBones) {
                    bone->startScaleTransform();
                    bone->scale(scaleXBy, scaleYBy);
                }
            } else {
                for(Bone *bone : mSelectedBones) {
                    bone->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                for(Bone *bone : mSelectedBones) {
                    bone->startScaleTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->scaleRelativeToSavedPivot(scaleXBy,
                                                    scaleYBy);
                }
            } else {
                for(Bone *bone : mSelectedBones) {
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
    for(const auto &box : mSelectedBoxes) {
        posSum += box->getPivotAbsPos();
    }
    return posSum/count;
}

bool Canvas::isSelectionEmpty() {
    return mSelectedBoxes.isEmpty();
}

void Canvas::ungroupSelectedBoxes() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isBoxesGroup()) {
            GetAsPtr(box, BoxesGroup)->ungroup_k();
        }
    }
}

void Canvas::centerPivotForSelected() {
    for(const auto &box : mSelectedBoxes) {
        box->centerPivotPosition();
    }
}

void Canvas::removeSelectedBoxesAndClearList() {
    for(const auto &box : mSelectedBoxes) {
        //box->deselect();
        box->removeFromParent_k();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void Canvas::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) {
        return;
    }
    connect(box, &BoundingBox::globalPivotInfluenced,
            this, &Canvas::schedulePivotUpdate);
    connect(box, &BoundingBox::fillStrokeSettingsChanged,
            this, &Canvas::scheduleDisplayedFillStrokeSettingsUpdate);
    box->select();
    mSelectedBoxes.append(box); schedulePivotUpdate();
    sortSelectedBoxesByZAscending();
    //setCurrentFillStrokeSettingsFromBox(box);
    mMainWindow->setCurrentBox(box);
}

void Canvas::removeBoxFromSelection(BoundingBox *box) {
    disconnect(box, &BoundingBox::globalPivotInfluenced,
               this, &Canvas::schedulePivotUpdate);
    disconnect(box, &BoundingBox::fillStrokeSettingsChanged,
               this, &Canvas::scheduleDisplayedFillStrokeSettingsUpdate);
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
    for(Bone *bone : mSelectedBones) {
        bone->deselect();
    }
    mSelectedBones.clear(); schedulePivotUpdate();
}

void Canvas::clearBoxesSelection() {
    for(const auto &box : mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
    mMainWindow->setCurrentBox(nullptr);
//    if(mLastPressedBox) {
//        mLastPressedBox->deselect();
//        mLastPressedBox = nullptr;
//    }
}


void Canvas::applyCurrentTransformationToSelected() {
    for(const auto &box : mSelectedBoxes) {
        box->applyCurrentTransformation();
    }
}

bool zLessThan(const qptr<BoundingBox> &box1, const qptr<BoundingBox> &box2) {
    return box1->getZIndex() > box2->getZIndex();
}

void Canvas::sortSelectedBoxesByZAscending() {
    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
}

void Canvas::raiseSelectedBoxesToTop() {
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        box->bringToFront();
    }
}

void Canvas::lowerSelectedBoxesToBottom() {
    for(const auto &box : mSelectedBoxes) {
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
    for(const auto &box : mSelectedBoxes) {
        int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) {
            box->moveUp();
        }
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::deselectAllBoxes() {
    for(const auto &box : mSelectedBoxes) {
        removeBoxFromSelection(box);
    }
}

MovablePoint *Canvas::getPointAtAbsPos(const QPointF &absPos,
                                 const CanvasMode &currentMode,
                                 const qreal &canvasScaleInv) {
    if(currentMode == MOVE_POINT ||
       currentMode == ADD_POINT ||
       currentMode == ADD_SMART_POINT ||
       currentMode == MOVE_PATH ||
       currentMode == ADD_BONE) {
        if(mRotPivot->isPointAtAbsPos(absPos, canvasScaleInv)) {
            return mRotPivot.get();
        }
        MovablePoint *pointAtPos = nullptr;
        for(const auto &box : mSelectedBoxes) {
            pointAtPos = box->getPointAtAbsPos(absPos,
                                               currentMode,
                                               canvasScaleInv);
            if(pointAtPos) {
                break;
            }
        }
        return pointAtPos;
    }
    return nullptr;
}

void Canvas::finishSelectedBoxesTransform() {
    if(mSelectedBones.isEmpty()) {
        for(const auto &box : mSelectedBoxes) {
            box->finishTransform();
        }
    } else {
        for(Bone *bone : mSelectedBones) {
            bone->finishTransform();
        }
    }
}

void Canvas::cancelSelectedBoxesTransform() {
    if(mSelectedBones.isEmpty()) {
        for(const auto &box : mSelectedBoxes) {
            box->cancelTransform();
        }
    } else {
        for(Bone *bone : mSelectedBones) {
            bone->cancelTransform();
        }
    }
}

void Canvas::moveSelectedBoxesByAbs(const QPointF &by,
                                    const bool &startTransform) {
    if(startTransform) {
        for(const auto &box : mSelectedBoxes) {
            box->startPosTransform();
            box->moveByAbs(by);
        }
    } else {
        for(const auto &box : mSelectedBoxes) {
            box->moveByAbs(by);
        }
    }
}

void Canvas::moveSelectedBonesByAbs(const QPointF &by,
                                    const bool &startTransform) {
    if(startTransform) {
        for(Bone *bone : mSelectedBones) {
            bone->startPosTransform();
            bone->moveByAbs(by);
        }
    } else {
        for(Bone *bone : mSelectedBones) {
            bone->moveByAbs(by);
        }
    }
}


//QPointF BoxesGroup::getRelCenterPosition() {
//    QPointF posSum = QPointF(0., 0.);
//    if(mChildren.isEmpty()) return posSum;
//    int count = mChildren.length();
//    for(const auto& box : mChildren) {
//        posSum += box->getPivotAbsPos();
//    }
//    return mapAbsPosToRel(posSum/count);
//}

#include "Boxes/linkbox.h"
void Canvas::createLinkBoxForSelected() {
    for(const auto& selectedBox : mSelectedBoxes) {
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
    target.write(rcChar(&nBoxes), sizeof(int));

    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    for(const auto &box : mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();

    clearBoxesSelection();
    container->pasteTo(mCurrentBoxesGroup);
}

void Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) return;
    auto newGroup = SPtrCreate(BoxesGroup)();
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        auto boxSP = GetAsSPtr(box, BoundingBox);
        box->removeFromParent_k();
        newGroup->addContainedBox(boxSP);
    }
    mCurrentBoxesGroup->addContainedBox(newGroup);
    mSelectedBoxes.clear(); schedulePivotUpdate();
    addBoxToSelection(newGroup.get());
}

VectorPath *Canvas::getPathResultingFromOperation(
        const SkPathOp& pathOp) {
    auto newPath = SPtrCreate(VectorPath)();
    SkOpBuilder builder;
    bool first = true;
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isPathBox()) {
            SkPath boxPath = GetAsPtr(box, PathBox)->getRelativePath();
            QMatrix boxTrans = box->getRelativeTransformAtCurrentFrame();
            boxPath.transform(QMatrixToSkMatrix(boxTrans));
            if(first) {
                builder.add(boxPath, SkPathOp::kUnion_SkPathOp);
                first = false;
            } else {
                builder.add(boxPath, pathOp);
            }
        }
    }
    SkPath resultingPath;
    builder.resolve(&resultingPath);
    newPath->loadPathFromSkPath(resultingPath);
    mCurrentBoxesGroup->addContainedBox(newPath);
    return newPath.get();
}

void Canvas::selectedPathsDifference() {
    VectorPath *newPath =
            getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsIntersection() {
    VectorPath *newPath = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsDivision() {
    VectorPath *newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    VectorPath *newPath2 = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}

void Canvas::selectedPathsExclusion() {
    VectorPath *newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);
    VectorPath *newPath2 = getPathResultingFromOperation(
                SkPathOp::kReverseDifference_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}


void Canvas::selectedPathsBreakApart() {
    if(mSelectedBoxes.isEmpty()) return;
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->breakPathsApart_k();
        }
    }
}

void Canvas::selectedPathsUnion() {
    VectorPath *newPath = getPathResultingFromOperation(
                SkPathOp::kUnion_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

#include "Animators/pathanimator.h"
void Canvas::selectedPathsCombine() {
    if(mSelectedBoxes.isEmpty()) return;
    VectorPath *firstVectorPath = nullptr;
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            firstVectorPath = GetAsPtr(box, VectorPath);
            break;
        }
    }
    if(!firstVectorPath) {
        auto newPath = SPtrCreate(VectorPath)();
        addContainedBox(newPath);
        firstVectorPath = newPath.get();
    }
    QMatrix firstTranf = firstVectorPath->getCombinedTransform();
    for(const auto &box : mSelectedBoxes) {
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
            box->removeFromParent_k();
        }
    }
}
