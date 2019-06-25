#include "actions.h"
#include "document.h"
#include "canvas.h"
#include "GUI/BrushWidgets/simplebrushwrapper.h"
#include "paintsettingsapplier.h"

Actions::Actions(Document &document) : mDocument(document),
    mActiveScene(mDocument.fActiveScene) {}

void Actions::raiseAction() const {
    if(!mActiveScene) return;
    mActiveScene->raiseSelectedBoxes();
    afterAction();
}

void Actions::lowerAction() const {
    if(!mActiveScene) return;
    mActiveScene->lowerSelectedBoxes();
    afterAction();
}

void Actions::raiseToTopAction() const {
    if(!mActiveScene) return;
    mActiveScene->raiseSelectedBoxesToTop();
    afterAction();
}

void Actions::lowerToBottomAction() const {
    if(!mActiveScene) return;
    mActiveScene->lowerSelectedBoxesToBottom();
    afterAction();
}

void Actions::objectsToPathAction() const {
    if(!mActiveScene) return;
    mActiveScene->convertSelectedBoxesToPath();
    afterAction();
}

void Actions::strokeToPathAction() const {
    if(!mActiveScene) return;
    mActiveScene->convertSelectedPathStrokesToPath();
    afterAction();
}

void Actions::rotate90CWAction() const {
    if(!mActiveScene) return;
    mActiveScene->rotateSelectedBoxesStartAndFinish(90);
    afterAction();
}

void Actions::rotate90CCWAction() const {
    if(!mActiveScene) return;
    mActiveScene->rotateSelectedBoxesStartAndFinish(-90);
    afterAction();
}

void Actions::flipHorizontalAction() const {
    if(!mActiveScene) return;
    mActiveScene->flipSelectedBoxesHorizontally();
    afterAction();
}

void Actions::flipVerticalAction() const {
    if(!mActiveScene) return;
    mActiveScene->flipSelectedBoxesVertically();
    afterAction();
}

void Actions::pathsUnionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsUnion();
    afterAction();
}

void Actions::pathsDifferenceAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsDifference();
    afterAction();
}

void Actions::pathsIntersectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsIntersection();
    afterAction();
}

void Actions::pathsDivisionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsDivision();
    afterAction();
}

void Actions::pathsExclusionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsExclusion();
    afterAction();
}

void Actions::pathsCombineAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsCombine();
    afterAction();
}

void Actions::pathsBreakApartAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsBreakApart();
    afterAction();
}

void Actions::setFontFamilyAndStyle(const QString& family,
                                    const QString& style) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedFontFamilyAndStyle(family, style);
    afterAction();
}

void Actions::setFontSize(const qreal size) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedFontSize(size);
    afterAction();
}

void Actions::connectPointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->connectPoints();
    afterAction();
}

void Actions::disconnectPointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->disconnectPoints();
    afterAction();
}

void Actions::mergePointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->mergePoints();
    afterAction();
}

void Actions::makePointCtrlsSymmetric() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsSymmetric();
    afterAction();
}

void Actions::makePointCtrlsSmooth() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsSmooth();
    afterAction();
}

void Actions::makePointCtrlsCorner() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsCorner();
    afterAction();
}

void Actions::makeSegmentLine() const {
    if(!mActiveScene) return;
    mActiveScene->makeSegmentLine();
    afterAction();
}

void Actions::makeSegmentCurve() const {
    if(!mActiveScene) return;
    mActiveScene->makeSegmentCurve();
    afterAction();
}

void Actions::startSelectedStrokeWidthTransform() const {
    if(!mActiveScene) return;
    mActiveScene->startSelectedStrokeWidthTransform();
    afterAction();
}

void Actions::deleteAction() const {
    if(!mActiveScene) return;
    mActiveScene->deleteAction();
}

void Actions::copyAction() const {
    if(!mActiveScene) return;
    mActiveScene->copyAction();
}

void Actions::pasteAction() const {
    if(!mActiveScene) return;
    mActiveScene->pasteAction();
}

void Actions::cutAction() const {
    if(!mActiveScene) return;
    mActiveScene->cutAction();
}

void Actions::duplicateAction() const {
    if(!mActiveScene) return;
    mActiveScene->duplicateAction();
}

void Actions::selectAllAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectAllAction();
}

void Actions::invertSelectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->invertSelectionAction();
}

void Actions::clearSelectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->clearSelectionAction();
}

void Actions::groupSelectedBoxes() const {
    if(!mActiveScene) return;
    mActiveScene->groupSelectedBoxes();
    afterAction();
}

void Actions::ungroupSelectedBoxes() const {
    if(!mActiveScene) return;
    mActiveScene->ungroupSelectedBoxes();
    afterAction();
}

void Actions::startSelectedStrokeColorTransform() const {
    if(!mActiveScene) return;
    mActiveScene->startSelectedStrokeColorTransform();
    afterAction();
}

void Actions::startSelectedFillColorTransform() const {
    if(!mActiveScene) return;
    mActiveScene->startSelectedFillColorTransform();
    afterAction();
}

void Actions::strokeCapStyleChanged(const SkPaint::Cap capStyle) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedCapStyle(capStyle);
    afterAction();
}

void Actions::strokeJoinStyleChanged(const SkPaint::Join joinStyle) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedJoinStyle(joinStyle);
    afterAction();
}

void Actions::strokeBrushChanged(SimpleBrushWrapper * const brush) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeBrush(brush);
    afterAction();
}

void Actions::strokeBrushWidthCurveChanged(
        const qCubicSegment1D& curve) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeBrushWidthCurve(curve);
    afterAction();
}

void Actions::strokeBrushTimeCurveChanged(
        const qCubicSegment1D& curve) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeBrushTimeCurve(curve);
    afterAction();
}

void Actions::strokeBrushSpacingCurveChanged(
        const qCubicSegment1D& curve) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeBrushSpacingCurve(curve);
    afterAction();
}

void Actions::strokeBrushPressureCurveChanged(
        const qCubicSegment1D& curve) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeBrushPressureCurve(curve);
    afterAction();
}

void Actions::strokeWidthChanged(const qreal strokeWidth) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeWidth(strokeWidth);
    afterAction();
}

void Actions::applyPaintSettingToSelected(
        const PaintSettingsApplier &setting) const {
    if(!mActiveScene) return;
    mActiveScene->applyPaintSettingToSelected(setting);
    afterAction();
}

void Actions::setSelectedFillColorMode(const ColorMode mode) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedFillColorMode(mode);
    afterAction();
}

void Actions::setSelectedStrokeColorMode(const ColorMode mode) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedStrokeColorMode(mode);
    afterAction();
}

void Actions::updateAfterFrameChanged(const int currentFrame) const {
    if(!mActiveScene) return;
    mActiveScene->anim_setAbsFrame(currentFrame);
    afterAction();
}

void Actions::afterAction() const {

}
