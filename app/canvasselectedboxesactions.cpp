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
    const bool thisHasNext = BoundingBox::anim_nextRelFrameWithKey(
                relFrame, thisNext);
    int minNextFrame = FrameRange::EMAX;
    for(const auto &box : mSelectedBoxes) {
        const int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxNext;
        if(box->anim_nextRelFrameWithKey(boxRelFrame, boxNext)) {
            const int absNext = box->prp_relFrameToAbsFrame(boxNext);
            if(minNextFrame > absNext) minNextFrame = absNext;
        }
    }
    if(minNextFrame == FrameRange::EMAX) {
        if(thisHasNext) nextRelFrame = thisNext;
        return thisHasNext;
    }
    if(thisHasNext) nextRelFrame = qMin(minNextFrame, thisNext);
    else nextRelFrame = minNextFrame;
    return true;
}

bool Canvas::anim_prevRelFrameWithKey(const int &relFrame,
                                     int &prevRelFrame) {
    int thisPrev;
    const bool thisHasPrev = BoundingBox::anim_prevRelFrameWithKey(
                relFrame, thisPrev);
    int minPrevFrame = FrameRange::EMIN;
    for(const auto &box : mSelectedBoxes) {
        const int boxRelFrame = box->prp_absFrameToRelFrame(relFrame);
        int boxPrev;
        if(box->anim_prevRelFrameWithKey(boxRelFrame, boxPrev)) {
            const int absPrev = box->prp_relFrameToAbsFrame(boxPrev);
            if(minPrevFrame < absPrev) minPrevFrame = absPrev;
        }
    }
    if(minPrevFrame == FrameRange::EMIN) {
        if(thisHasPrev) prevRelFrame = thisPrev;
        return thisHasPrev;
    }
    if(thisHasPrev) prevRelFrame = qMax(minPrevFrame, thisPrev);
    else prevRelFrame = minPrevFrame;
    return true;
}

void Canvas::shiftAllPointsForAllKeys(const int &by) {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isVectorPath()) {
            GetAsPtr(box, VectorPath)->shiftAllPointsForAllKeys(by);
        }
    }
}

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
        box->scale(-1, 1);
        box->finishTransform();
    }
}

void Canvas::flipSelectedBoxesVertically() {
    for(const auto &box : mSelectedBoxes) {
        box->startScaleTransform();
        box->scale(1, -1);
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
        box->addPathEffect(SPtrCreate(SumPathEffect)(false));
    }
}

void Canvas::applyOperationPathEffectToSelected() {
    for(const auto &box : mSelectedBoxes) {
        if(!box->SWT_isPathBox()) continue;
        const auto pathBox = GetAsPtr(box, PathBox);
        box->addPathEffect(SPtrCreate(OperationPathEffect)(pathBox, false));
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
        const auto pathBox = GetAsPtr(box, PathBox);
        const auto effect = SPtrCreate(OperationPathEffect)(pathBox, false);
        box->addFillPathEffect(effect);
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
    for(const auto &box : mSelectedBoxes)
        box->resetTranslation();
}

void Canvas::resetSelectedScale() {
    for(const auto &box : mSelectedBoxes)
        box->resetScale();
}

void Canvas::resetSelectedRotation() {
    for(const auto &box : mSelectedBoxes)
        box->resetRotation();
}

void Canvas::getDisplayedFillStrokeSettingsFromLastSelected(
        PaintSettingsAnimator*& fillSetings, OutlineSettingsAnimator*& strokeSettings) {
    if(mSelectedBoxes.isEmpty()) {
        fillSetings = nullptr;
        strokeSettings = nullptr;
    } else {
        const auto box = mSelectedBoxes.last();
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

VectorPathEdge *Canvas::getEdgeAt(const QPointF& absPos) const {
    for(const auto &box : mSelectedBoxes) {
        const qreal zoomInv = 1/mCanvasTransformMatrix.m11();
        const auto pathEdge = box->getEdge(absPos, zoomInv);
        if(!pathEdge) return pathEdge;
    }
    return nullptr;
}
#include "Boxes/smartvectorpath.h"
NormalSegment Canvas::getSmartEdgeAt(const QPointF& absPos) const {
    for(const auto &box : mSelectedBoxes) {
        const qreal zoomInv = 1/mCanvasTransformMatrix.m11();
        const auto pathEdge = box->getNormalSegment(absPos, zoomInv);
        if(pathEdge.isValid()) return pathEdge;
    }
    return NormalSegment();
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
                for(const auto& bone : mSelectedBones) {
                    bone->startRotTransform();
                    bone->rotateBy(rotBy);
                }
            } else {
                for(const auto& bone : mSelectedBones) {
                    bone->rotateBy(rotBy);
                }
            }
        } else {
            if(startTrans) {
                for(const auto& bone : mSelectedBones) {
                    bone->startRotTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->rotateRelativeToSavedPivot(rotBy);
                }
            } else {
                for(const auto& bone : mSelectedBones) {
                    bone->rotateRelativeToSavedPivot(rotBy);
                }
            }
        }
    }
}

void Canvas::scaleSelectedBy(const qreal &scaleBy,
                             const QPointF &absOrigin,
                             const bool &startTrans) {
    scaleSelectedBy(scaleBy, scaleBy, absOrigin, startTrans);
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
                for(const auto& bone : mSelectedBones) {
                    bone->startScaleTransform();
                    bone->scale(scaleXBy, scaleYBy);
                }
            } else {
                for(const auto& bone : mSelectedBones) {
                    bone->scale(scaleXBy, scaleYBy);
                }
            }
        } else {
            if(startTrans) {
                for(const auto& bone : mSelectedBones) {
                    bone->startScaleTransform();
                    bone->startPosTransform();
                    bone->saveTransformPivotAbsPos(absOrigin);
                    bone->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
                }
            } else {
                for(const auto& bone : mSelectedBones) {
                    bone->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
                }
            }
        }
    }
}

QPointF Canvas::getSelectedBoxesAbsPivotPos() {
    if(mSelectedBoxes.isEmpty()) return QPointF(0, 0);
    QPointF posSum = QPointF(0, 0);
    const int count = mSelectedBoxes.length();
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
    for(const auto &box : mSelectedBoxes)
        box->centerPivotPosition();
}

void Canvas::removeSelectedBoxesAndClearList() {
    for(const auto &box : mSelectedBoxes) {
        //box->deselect();
        box->removeFromParent_k();
    }
    mSelectedBoxes.clear(); schedulePivotUpdate();
}

void Canvas::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) return;
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
    if(bone->isSelected()) return;
    bone->select();
    mSelectedBones.append(bone); schedulePivotUpdate();
    //sortSelectedBonesByZAscending();
}

void Canvas::removeBoneFromSelection(Bone *bone) {
    bone->deselect();
    mSelectedBones.removeOne(bone); schedulePivotUpdate();
}

void Canvas::clearBonesSelection() {
    for(const auto& bone : mSelectedBones)
        bone->deselect();
    mSelectedBones.clear(); schedulePivotUpdate();
}

void Canvas::clearBoxesSelection() {
    for(const auto &box : mSelectedBoxes)
        box->deselect();
    mSelectedBoxes.clear(); schedulePivotUpdate();
    mMainWindow->setCurrentBox(nullptr);
//    if(mLastPressedBox) {
//        mLastPressedBox->deselect();
//        mLastPressedBox = nullptr;
//    }
}


void Canvas::applyCurrentTransformationToSelected() {
    for(const auto &box : mSelectedBoxes)
        box->applyCurrentTransformation();
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
    for(const auto &box : mSelectedBoxes)
        box->bringToEnd();
}

void Canvas::lowerSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        const int boxZ = box->getZIndex();
        if(boxZ - 1 != lastZ || lastBoxChanged) box->moveDown();
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::raiseSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    for(const auto &box : mSelectedBoxes) {
        const int boxZ = box->getZIndex();
        if(boxZ + 1 != lastZ || lastBoxChanged) box->moveUp();
        lastZ = boxZ;
        lastBoxChanged = boxZ - box->getZIndex() != 0;
    }
}

void Canvas::deselectAllBoxes() {
    for(const auto &box : mSelectedBoxes)
        removeBoxFromSelection(box);
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
            pointAtPos = box->getPointAtAbsPos(absPos, currentMode,
                                               canvasScaleInv);
            if(pointAtPos) break;
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
        for(const auto& bone : mSelectedBones) {
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
        for(const auto& bone : mSelectedBones) {
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
        for(const auto& bone : mSelectedBones) {
            bone->startPosTransform();
            bone->moveByAbs(by);
        }
    } else {
        for(const auto& bone : mSelectedBones) {
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
    const int nBoxes = mSelectedBoxes.count();
    target.write(rcConstChar(&nBoxes), sizeof(int));

    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), boxesZSort);
    for(const auto &box : mSelectedBoxes) {
        box->writeBoundingBox(&target);
    }
    target.close();
    BoundingBox::sClearWriteBoxes();

    clearBoxesSelection();
    container->pasteTo(mCurrentBoxesGroup);
}

void Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.count() == 0) return;
    const auto newGroup = SPtrCreate(BoxesGroup)();
    BoundingBox* box;
    Q_FOREACHInverted(box, mSelectedBoxes) {
        const auto boxSP = GetAsSPtr(box, BoundingBox);
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
            const QMatrix boxTrans = box->getRelativeTransformAtCurrentFrame();
            boxPath.transform(toSkMatrix(boxTrans));
            if(first) {
                builder.add(boxPath, SkPathOp::kUnion_SkPathOp);
                first = false;
            } else {
                builder.add(boxPath, pathOp);
            }
        }
    }
    SkPath resultPath;
    builder.resolve(&resultPath);
    newPath->loadPathFromSkPath(resultPath);
    mCurrentBoxesGroup->addContainedBox(newPath);
    return newPath.get();
}

void Canvas::selectedPathsDifference() {
    VectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsIntersection() {
    VectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsDivision() {
    VectorPath * const newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    VectorPath * const newPath2 = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}

void Canvas::selectedPathsExclusion() {
    VectorPath * const newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);
    VectorPath * const newPath2 = getPathResultingFromOperation(
                SkPathOp::kReverseDifference_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}


void Canvas::selectedPathsBreakApart() {
    if(mSelectedBoxes.isEmpty()) return;
    QList<qsptr<SmartVectorPath>> created;
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
            const auto path = GetAsPtr(box, SmartVectorPath);
            created << path->breakPathsApart_k();
        }
    }
    for(const auto& path : created) {
        mCurrentBoxesGroup->addContainedBox(path);
        addBoxToSelection(path.get());
    }
}

void Canvas::selectedPathsUnion() {
    VectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kUnion_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

#include "Animators/pathanimator.h"
void Canvas::selectedPathsCombine() {
    if(mSelectedBoxes.isEmpty()) return;
    SmartVectorPath *firstVectorPath = nullptr;
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
            firstVectorPath = GetAsPtr(box, SmartVectorPath);
            break;
        }
    }
    if(!firstVectorPath) {
        const auto newPath = SPtrCreate(SmartVectorPath)();
        mCurrentBoxesGroup->addContainedBox(newPath);
        firstVectorPath = newPath.get();
    }
    const auto targetVP = firstVectorPath->getHandler();
    const QMatrix firstTranf = firstVectorPath->getTotalTransform();
    for(const auto &box : mSelectedBoxes) {
        if(box == firstVectorPath) continue;
        if(box->SWT_isSmartVectorPath()) {
            const auto boxPath = GetAsPtr(box, SmartVectorPath);
            const QMatrix relTransf = boxPath->getTotalTransform()*
                    firstTranf.inverted();
            const auto srcVP = boxPath->getHandler();
            srcVP->applyTransform(relTransf);
            targetVP->moveAllFrom(srcVP);
            box->removeFromParent_k();
        }
    }
}
