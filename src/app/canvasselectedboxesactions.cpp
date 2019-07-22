#include "canvas.h"
#include "GUI/mainwindow.h"
#include "MovablePoints/pathpivot.h"
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
#include "Boxes/smartvectorpath.h"
#include "Animators/SmartPath/smartpathcollection.h"

void Canvas::groupSelectedBoxes() {
    if(mSelectedBoxes.isEmpty()) return;
    const auto newGroup = SPtrCreate(ContainerBox)(TYPE_GROUP);
    mCurrentContainer->addContainedBox(newGroup);
    for(int i = mSelectedBoxes.count() - 1; i >= 0; i--) {
        const auto boxSP = GetAsSPtr(mSelectedBoxes.at(i), BoundingBox);
        boxSP->removeFromParent_k();
        newGroup->addContainedBox(boxSP);
    }
    clearBoxesSelectionList();
    newGroup->planCenterPivotPosition();
    schedulePivotUpdate();
    addBoxToSelection(newGroup.get());
}

bool Canvas::anim_nextRelFrameWithKey(const int relFrame,
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

bool Canvas::anim_prevRelFrameWithKey(const int relFrame,
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

void Canvas::shiftAllPointsForAllKeys(const int by) {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
//            const auto svp = GetAsPtr(box, SmartVectorPath);
//            svp->shiftAllPointsForAllKeys(by);
        }
    }
}

void Canvas::revertAllPointsForAllKeys() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
            //GetAsPtr(box, SmartVectorPath)->revertAllPointsForAllKeys();
        }
    }
}

void Canvas::shiftAllPoints(const int by) {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
            //GetAsPtr(box, SmartVectorPath)->shiftAllPoints(by);
        }
    }
}

void Canvas::revertAllPoints() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isSmartVectorPath()) {
            //GetAsPtr(box, SmartVectorPath)->revertAllPoints();
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

void Canvas::setSelectedFontSize(const qreal size) {
    for(const auto &box : mSelectedBoxes) {
        box->setSelectedFontSize(size);
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

void Canvas::setSelectedFillColorMode(const ColorMode mode) {
    for(const auto &box : mSelectedBoxes) {
        box->setFillColorMode(mode);
    }
}

void Canvas::setSelectedStrokeColorMode(const ColorMode mode) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeColorMode(mode);
    }
}

void Canvas::setSelectedCapStyle(const SkPaint::Cap capStyle) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeCapStyle(capStyle);
    }
}

void Canvas::setSelectedJoinStyle(const SkPaint::Join joinStyle) {
    for(const auto &box : mSelectedBoxes) {
        box->setStrokeJoinStyle(joinStyle);
    }
}

void Canvas::strokeWidthAction(const QrealAction& action) {
    for(const auto &box : mSelectedBoxes)
        box->strokeWidthAction(action);
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

#include "Boxes/smartvectorpath.h"
NormalSegment Canvas::getSegment(const MouseEvent& e) const {
    const qreal zoomInv = 1/e.fScale;
    for(const auto &box : mSelectedBoxes) {
        const auto pathEdge = box->getNormalSegment(e.fPos, zoomInv);
        if(pathEdge.isValid()) return pathEdge;
    }
    return NormalSegment();
}

void Canvas::rotateSelectedBoxesStartAndFinish(const qreal rotBy) {
    if(mDocument.fLocalPivot) {
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

void Canvas::rotateSelectedBy(const qreal rotBy,
                              const QPointF &absOrigin,
                              const bool startTrans) {
    if(mDocument.fLocalPivot) {
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
}

void Canvas::scaleSelectedBy(const qreal scaleBy,
                             const QPointF &absOrigin,
                             const bool startTrans) {
    scaleSelectedBy(scaleBy, scaleBy, absOrigin, startTrans);
}

void Canvas::scaleSelectedBy(const qreal scaleXBy,
                             const qreal scaleYBy,
                             const QPointF& absOrigin,
                             const bool startTrans) {
    if(mDocument.fLocalPivot) {
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
                box->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            for(const auto &box : mSelectedBoxes) {
                box->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    }
}

QPointF Canvas::getSelectedBoxesAbsPivotPos() {
    if(mSelectedBoxes.isEmpty()) return QPointF(0, 0);
    QPointF posSum(0, 0);
    const int count = mSelectedBoxes.count();
    for(const auto &box : mSelectedBoxes)
        posSum += box->getPivotAbsPos();
    return posSum/count;
}

bool Canvas::isBoxSelectionEmpty() const {
    return mSelectedBoxes.isEmpty();
}

void Canvas::ungroupSelectedBoxes() {
    for(const auto &box : mSelectedBoxes) {
        if(box->SWT_isContainerBox()) {
            GetAsPtr(box, ContainerBox)->ungroup_k();
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
    clearBoxesSelectionList(); schedulePivotUpdate();
}

void Canvas::setCurrentBox(BoundingBox* const box) {
    mCurrentBox = box;
    emit currentBoxChanged(box);
}

#include "Boxes/paintbox.h"
void Canvas::addBoxToSelection(BoundingBox * const box) {
    if(box->isSelected()) return;
    auto& connCtx = mSelectedBoxes.addObj(box);
    connCtx << connect(box, &BoundingBox::globalPivotInfluenced,
                       this, &Canvas::schedulePivotUpdate);
    connCtx << connect(box, &BoundingBox::fillStrokeSettingsChanged,
                       this, &Canvas::selectedPaintSettingsChanged);
    connCtx << connect(box, &BoundingBox::visibilityChanged,
                       this, [this, box](const bool visible) {
        if(!visible) removeBoxFromSelection(box);
    });
    connCtx << connect(box, &BoundingBox::parentChanged,
                       this, [this, box](const bool visible) {
        if(!visible) removeBoxFromSelection(box);
    });

    box->select();
    schedulePivotUpdate();

    sortSelectedBoxesAsc();
    //setCurrentFillStrokeSettingsFromBox(box);
    setCurrentBox(box);

    if(mCurrentMode == PAINT_MODE) {
        if(box->SWT_isPaintBox()) mPaintTarget.setPaintBox(GetAsPtr(box, PaintBox));
    }
    emit selectedPaintSettingsChanged();
}

void Canvas::removeBoxFromSelection(BoundingBox * const box) {
    if(!box->isSelected()) return;
    mSelectedBoxes.removeObj(box);
    box->deselect();
    schedulePivotUpdate();
    if(mCurrentMode == PAINT_MODE) updatePaintBox();
    if(mSelectedBoxes.isEmpty()) {
        setCurrentBox(nullptr);
    } else {
        setCurrentBox(mSelectedBoxes.last());
    }
    emit selectedPaintSettingsChanged();
}

void Canvas::clearBoxesSelection() {
    for(const auto &box : mSelectedBoxes)
        box->deselect();
    clearBoxesSelectionList();
    schedulePivotUpdate();
    setCurrentBox(nullptr);
//    if(mLastPressedBox) {
//        mLastPressedBox->deselect();
//        mLastPressedBox = nullptr;
    //    }
}

void Canvas::clearBoxesSelectionList() {
    if(mCurrentMode == PAINT_MODE) mPaintTarget.setPaintBox(nullptr);
    mSelectedBoxes.clear();
    emit selectedPaintSettingsChanged();
}

void Canvas::applyCurrentTransformationToSelected() {
}

bool zAsc(const qptr<BoundingBox> &box1, const qptr<BoundingBox> &box2) {
    return box1->getZIndex() > box2->getZIndex();
}

void Canvas::sortSelectedBoxesAsc() {
    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zAsc);
}

bool zDesc(const qptr<BoundingBox>& box1, const qptr<BoundingBox>& box2) {
    return box1->getZIndex() < box2->getZIndex();
}

void Canvas::sortSelectedBoxesDesc() {
    std::sort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zDesc);
}

void Canvas::raiseSelectedBoxesToTop() {
    const auto begin = mSelectedBoxes.rbegin();
    const auto end = mSelectedBoxes.rend();
    for(auto it = begin; it != end; it++) {
        (*it)->bringToFront();
    }
}

void Canvas::lowerSelectedBoxesToBottom() {
    for(const auto &box : mSelectedBoxes)
        box->bringToEnd();
}

void Canvas::lowerSelectedBoxes() {
    int lastZ = -10000;
    bool lastBoxChanged = true;
    const auto begin = mSelectedBoxes.rbegin();
    const auto end = mSelectedBoxes.rend();
    for(auto it = begin; it != end; it++) {
        const auto box = *it;
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
                                       const CanvasMode mode,
                                       const qreal invScale) {
    if(mode == MOVE_POINT || mode == ADD_POINT ||
       mode == ADD_POINT ||  mode == MOVE_BOX) {
        if(mRotPivot->isPointAtAbsPos(absPos, mode, invScale)) {
            return mRotPivot.get();
        }
        for(const auto &box : mSelectedBoxes) {
            const auto pointAtPos = box->getPointAtAbsPos(absPos, mode, invScale);
            if(pointAtPos) return pointAtPos;
        }
    }
    return nullptr;
}

void Canvas::finishSelectedBoxesTransform() {
    for(const auto &box : mSelectedBoxes) {
        box->finishTransform();
    }
}

void Canvas::cancelSelectedBoxesTransform() {
    for(const auto &box : mSelectedBoxes) {
        box->cancelTransform();
    }
}

void Canvas::moveSelectedBoxesByAbs(const QPointF &by,
                                    const bool startTransform) {
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
        mCurrentContainer->addContainedBox(selectedBox->createLink());
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

    for(const auto &box : mSelectedBoxes) {
        box->writeBoxType(&target);
        box->writeBoundingBox(&target);
    }
    target.close();
    BoundingBox::sClearWriteBoxes();

    clearBoxesSelection();
    container->pasteTo(mCurrentContainer);
}

SmartVectorPath *Canvas::getPathResultingFromOperation(
        const SkPathOp& pathOp) {
    auto newPath = SPtrCreate(SmartVectorPath)();
    newPath->planCenterPivotPosition();
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
    //newPath->loadPathFromSkPath(resultPath);
    mCurrentContainer->addContainedBox(newPath);
    return newPath.get();
}

void Canvas::selectedPathsDifference() {
    SmartVectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsIntersection() {
    SmartVectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

void Canvas::selectedPathsDivision() {
    SmartVectorPath * const newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);

    SmartVectorPath * const newPath2 = getPathResultingFromOperation(
                SkPathOp::kIntersect_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath1);
    addBoxToSelection(newPath2);
}

void Canvas::selectedPathsExclusion() {
    SmartVectorPath * const newPath1 = getPathResultingFromOperation(
                SkPathOp::kDifference_SkPathOp);
    SmartVectorPath * const newPath2 = getPathResultingFromOperation(
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
        mCurrentContainer->addContainedBox(path);
        addBoxToSelection(path.get());
    }
}

void Canvas::selectedPathsUnion() {
    SmartVectorPath * const newPath = getPathResultingFromOperation(
                SkPathOp::kUnion_SkPathOp);

    clearBoxesSelection();
    addBoxToSelection(newPath);
}

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
        newPath->planCenterPivotPosition();
        mCurrentContainer->addContainedBox(newPath);
        firstVectorPath = newPath.get();
    }

    const auto targetVP = firstVectorPath->getPathAnimator();
    const QMatrix firstTranf = firstVectorPath->getTotalTransform();
    for(const auto &box : mSelectedBoxes) {
        if(box == firstVectorPath) continue;
        if(box->SWT_isSmartVectorPath()) {
            const auto boxPath = GetAsPtr(box, SmartVectorPath);
            const QMatrix relTransf = boxPath->getTotalTransform()*
                    firstTranf.inverted();
            const auto srcVP = boxPath->getPathAnimator();
            srcVP->applyTransform(relTransf);
            targetVP->moveAllFrom(srcVP);
            box->removeFromParent_k();
        }
    }
}
